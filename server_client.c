#include "server.h"
#include <time.h>
#include <stdlib.h>

#define DEFAULT_ROOM "Lobby"
#define EDT (-5)

time_t curtime;
struct tm *loc_time;

extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct USER *head;
extern struct ROOM *roomhead;
extern struct USERSINROOMS *usersinroomhead;

extern char *server_MOTD;
extern bool isSingleMode;

/*
 *Main thread for each client.  Receives all messages
 *and passes the data off to the correct function.  Receives
 *a pointer to the file descriptor for the socket the thread
 *should listen on
 */

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void *client_receive(void *ptr)
{
   int client = *(int *) ptr;
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  //data buffer of 4K  
   char tmpbuf[MAXBUFF];  //data temp buffer of 1K  
   char cmd[MAXBUFF], username[20];
   char *arguments[80];
   bool loggedIn = false;
   struct USER *multicastlist = NULL;

   struct USER *currentUser, *dmUser, *userList;
   struct USERSINROOMS *foundRoom;
   struct ROOM *simulcastList = NULL;

   send(client, server_MOTD , strlen(server_MOTD) , 0 ); // Send Welcome Message of the Day.

   sprintf(username,"guest%d", client);
   head = insertFirstU(head, client , username);
   currentUser = head; // new node is always at front of list.  findSocketU(head, client);
   currentUser->head = insertFirstRoom(currentUser->head, DEFAULT_ROOM);

   
  
   foundRoom = findUsersInRooms(usersinroomhead, DEFAULT_ROOM);

   if(foundRoom != NULL) 
   {
      foundRoom->head = insertFirstU(foundRoom->head, client ,currentUser->username);
   }
   else
   {
      usersinroomhead = insertFirstUsersInRooms(usersinroomhead, DEFAULT_ROOM);
      foundRoom = usersinroomhead;  // findUsersInRooms(usersinroomhead, DEFAULT_ROOM);
      foundRoom->head = insertFirstU(foundRoom->head, client ,currentUser->username);
   }

  
   while (1) {
       
      if ((received = read(client , buffer, MAXBUFF)) != 0) 
      {
      
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);
         
            /////////////////////////////////////////////////////
            // we got some data from a client

            // TODO:
            // 1. Tokenize the input in buf (split it on whitespace)

            // get the first token 

             arguments[0] = strtok(cmd, delimiters);

            // walk through other tokens 

             i = 0;
             while( arguments[i] != NULL ) 
             {
                arguments[++i] = strtok(NULL, delimiters); 
                strcpy(arguments[i-1], trimwhitespace(arguments[i-1]));
             } 

             /////////////////////////////////////////////////////
             // 2. Execute command:
        //STRETCH FEATURE: IF USER JOINS ROOM THAT DOES NOT EXIST THAT ROOM IS AUTOMATICALLY CREATED 

         
          if(strcmp(arguments[0], "create") == 0)
          {
            if(arguments[1] != NULL)
            {
              usersinroomhead = insertFirstUsersInRooms(usersinroomhead, arguments[1]);
            }
            // STRETCH: Timestamp for Creating Room
						time(&curtime); 
            loc_time = gmtime(&curtime);
            printf("create room: %s\n", arguments[1]);
            sprintf(buffer, "\nRoom created at %2d:%02d\nchat>", (loc_time->tm_hour + EDT) % 24, loc_time->tm_min);
            send(client , buffer , strlen(buffer) , 0 ); // send back to client
          }
            
            else if (strcmp(arguments[0], "join") == 0)
            {
                pthread_mutex_lock(&rw_lock); /* apply lock for join only, 
                limit read/write access to prevent race conditions */
                printf("User: %s Has Joined Room: %s\n", username, arguments[1]);  
                if(arguments[1] != NULL) 
                  {
                    currentUser = findSocketU(head, client);
                    foundRoom = findUsersInRooms(usersinroomhead, arguments[1]);
                  
                    if(foundRoom != NULL)
                    {
                    if(isSingleMode && currentUser != NULL)
                      {
                        currentUser->head = deleteFirstRoom(currentUser->head);
                        if(foundRoom != NULL) 
                          {
                           foundRoom->head = deleteU(foundRoom->head, currentUser->username);
                          }
                      } 
          

                    if(currentUser != NULL)
                      {
                        currentUser->head = insertFirstRoom(currentUser->head, arguments[1]);   
                        if(foundRoom != NULL) 
                          {
                           foundRoom->head = insertFirstU(foundRoom->head, client ,currentUser->username);
                          } else {
                            printf("Room does not exist. Creating room...");
                          
                        }//else statement was originally here 
                      }
                    }
                      
                    }
                  
                  
                pthread_mutex_unlock(&rw_lock); //unlock 
                sprintf(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }
            else if (strcmp(arguments[0], "leave") == 0)
            {
                printf("User: %s Has Left The Room: %s\n",username , arguments[1]);      
                pthread_mutex_lock(&rw_lock);
                if(arguments[1] != NULL)
                {
                  foundRoom = findUsersInRooms(usersinroomhead, arguments[1]);
                  if(foundRoom != NULL) 
                  {
                    currentUser = findSocketU(head, client);
                    foundRoom->head = deleteU(foundRoom->head, currentUser->username);
                    currentUser->head = deleteRoom(currentUser->head, arguments[1]);
                  }
                }
                pthread_mutex_unlock(&rw_lock);
                
                sprintf(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client
            } 
            else if (strcmp(arguments[0], "connect") == 0)
            {
                printf(">>>>>>User: %s Has Connected To User: %s \n", username, arguments[1]);
                pthread_mutex_lock(&rw_lock);
                if(isSingleMode)
                  {
                    currentUser = findSocketU(head, client);
                    currentUser->dmhead = deleteFirstU(currentUser->dmhead);
                    dmUser = findU(head, arguments[1]);
                    dmUser->dmhead = deleteFirstU(dmUser->dmhead);
                  }
               currentUser = findSocketU(head, client);
               if(currentUser != NULL) 
               {
                dmUser = findU(head, arguments[1]);
                if(dmUser != NULL)
                  {
                    currentUser->dmhead = insertFirstU(currentUser->dmhead, dmUser->socket, dmUser->username);
                    dmUser->dmhead = insertFirstU(dmUser->dmhead, currentUser->socket, currentUser->username);
                  }
                }
               pthread_mutex_unlock(&rw_lock);
               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }
            else if (strcmp(arguments[0], "priv_room") == 0){
              printf("Placeholder");
          
          
            }
            else if (strcmp(arguments[0], "disconnect") == 0)
            {             
               
               pthread_mutex_lock(&rw_lock);
               currentUser = findSocketU(head, client);
               if(currentUser != NULL) 
               {
                  dmUser = findU(head, arguments[1]);
                  if(dmUser != NULL)
                    {
                        currentUser->dmhead = deleteU(currentUser->dmhead, dmUser->username);
                        dmUser->dmhead = deleteU(dmUser->dmhead, currentUser->username);
                    }
               }
               pthread_mutex_unlock(&rw_lock); 
               printf(">>>>>>User: %s Has Disconnected From User: %s\n", username, arguments[1]);
               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 ); // send back to client
             }                  
            else if (strcmp(arguments[0], "rooms") == 0)
            {
                pthread_mutex_lock(&mutex);
                
                if(numReaders == 0)
                {
                  pthread_mutex_lock(&rw_lock);
                }
                numReaders += 1;
                pthread_mutex_unlock(&mutex);
              
                ////Server Side////
              
                printListUsersInRooms(usersinroomhead);
                ////////CS////////////
                display_usersinrooms(usersinroomhead, buffer);
                /////////////////////
                
                pthread_mutex_lock(&mutex);
                numReaders -= 1;
                if(numReaders == 0)
                {
                  pthread_mutex_unlock(&rw_lock);
                }
                pthread_mutex_unlock(&mutex);
              
                strcat(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client                            
            }   
            else if (strcmp(arguments[0], "users") == 0)
            {
                
              pthread_mutex_lock(&mutex);
              if(numReaders == 0)
              {
                  pthread_mutex_lock(&rw_lock);
              }
              numReaders += 1;  
              pthread_mutex_unlock(&mutex);
              
              ////Server Side////
               //printListU(head);
              
              
              //////////////// CS ///////////////////
               currentUser = findSocketU(head, client);
               dmUser = currentUser->dmhead;
               if(dmUser != NULL) 
               {
                 
                 sprintf(buffer, "[DM USERS<");

                 while(dmUser != NULL)
                 {
                     sprintf(tmpbuf,"(%s)", dmUser->username);
                  
                     strcat(buffer, tmpbuf);
                     dmUser = dmUser->next;
                     if(dmUser != NULL)
                     {
                       sprintf(tmpbuf, ", ");
                       strcat(buffer, tmpbuf);
                     }
                 }
                 sprintf(tmpbuf,">]\n");
                 strcat(buffer, tmpbuf);
               }
               simulcastList = currentUser->head;
               if(simulcastList != NULL) 
               {
                 while(simulcastList != NULL)
                 {
                       printf("ROOM: %s\n",simulcastList->roomname);
                       foundRoom = findUsersInRooms(usersinroomhead, simulcastList->roomname);
                        if(foundRoom == NULL)
                        {

                        }
                        else
                        {

                        }
                       
                       // loop through all the users/socks in the group 
                       userList = foundRoom->head;

                       sprintf(tmpbuf, "[ROOM: %s USERS<", foundRoom->roomname);
                  
                       strcat(buffer, tmpbuf);
                       while(userList != NULL)
                       {

                         if(userList->socket != client)
                         { // its not myself

                         sprintf(tmpbuf, "(%s)", userList->username);
                    
                         strcat(buffer, tmpbuf);

                         if(userList->next != NULL)
                           {
                             sprintf(tmpbuf, ", ");
                             strcat(buffer, tmpbuf);
                           }
                         }
                         userList = userList->next;
                       }

                       simulcastList = simulcastList->next;
                 
                       sprintf(tmpbuf, ">]\n");
                       strcat(buffer, tmpbuf);
                   }
               }
               else 
               {
                  display_users(head, buffer, client);
               }
               ///////////////////////////////////////////
                
                pthread_mutex_lock(&mutex);
                numReaders -= 1;
                if(numReaders == 0)
                {
                  pthread_mutex_unlock(&rw_lock);
                }
                pthread_mutex_unlock(&mutex);
                
                strcat(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 ); // send back to client
            }                           
            else if (strcmp(arguments[0], "login") == 0) // login  (anon user)
            {      
              pthread_mutex_lock(&mutex);
               if(arguments[1] != NULL && !loggedIn)
               {     
                    currentUser = findU(head, username);  // find the guest
                    if(currentUser != NULL) 
                    {
                      strcpy(currentUser->username, arguments[1]);
                      foundRoom = findUsersInRooms(usersinroomhead, DEFAULT_ROOM);
                      if(foundRoom != NULL) 
                      {
                        currentUser = findU(foundRoom->head,username);                
                        strcpy(currentUser->username, arguments[1]);
                        //username = currentUser->username;
                      }
                      loggedIn = true;
                      printListU(head);
                    }
               }
              printf("User: %s Has Logged In As %s\n", username , currentUser->username );
              pthread_mutex_unlock(&mutex);
              
              sprintf(buffer, "\nchat>");
              send(client , buffer , strlen(buffer) , 0 ); // send back to client
            } 

            else if (strcmp(arguments[0], "help") == 0 )
            {
              sprintf(buffer, "login <username> - \"login with username\" \ncreate <room> - \"create a room\" \njoin <room> - \"join a room\" \nleave <room> - \"leave a room\" \nusers - \"list all users\" \nrooms -  \"list all rooms\" \nconnect <user> - \"connect to user\" \nexit - \"exit chat\" \n");
              send(client , buffer , strlen(buffer) , 0 ); // send back to client 
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0)
            {
                pthread_mutex_lock(&mutex);
                //////////////// CS ///////////////////
                currentUser = findSocketU(head, client);
                close(currentUser->socket);
                ///////////////////////////////////////////
                
                pthread_mutex_unlock(&mutex);
                
            }
            else /// MESSAGING
            { 
            /////////////////////////////////////////////////////
            // 3. sending a message
          
                // Limit readers by 1 at a Time
                pthread_mutex_lock(&mutex);
                if(numReaders == 0)
                {
                  pthread_mutex_lock(&rw_lock);
                }
                numReaders += 1;
                pthread_mutex_unlock(&mutex);
                // end of lock ////
              

                printf("Message From User %s: %s", username,buffer); /// only shows in terminal
                //////////////// CS ///////////////////
                 currentUser = findSocketU(head, client);
                 
                printf("%s", arguments[0]);
                 // STRETCH Text to Emoji
                 if (strcmp(arguments[0],"!sticky") == 0){ //shouldnt this be arguments[0] because its user input
                   sprintf(tmpbuf,"\n::%s> %s\nchat>", currentUser->username, sbuffer);
                   strcpy(sbuffer, "O-|---<");
                 } 

                 else {
                   sprintf(tmpbuf,"\n::%s> %s\nchat>", currentUser->username, sbuffer);
                   strcpy(sbuffer, tmpbuf);
                 }
              
                 
                 // Send DMs first
              
                 // Now we Know Current User. We Dm Who they're connected to.
                 userList = currentUser->dmhead;
                 while(userList != NULL)
                 {
                   if(client != userList->socket) 
                   {  // check if less than max_clients
                      multicastlist = insertFirstU(multicastlist, userList->socket ,userList->username);
                   }
                   userList = userList->next;
                 }
              
                 // Send to Rooms Next
                 simulcastList = currentUser->head;
                 while (simulcastList != NULL)
                 {
                   // get linked list for every room
                   foundRoom = findUsersInRooms(usersinroomhead, simulcastList->roomname);
                   // loop through all the users/sockets in the group 
                   userList = foundRoom->head;
                   while(userList != NULL)
                   {
                     if(client != userList->socket)
                     {  // check if less than max_clients
                       multicastlist = insertFirstU(multicastlist, userList->socket ,userList->username);
                     }
                     userList = userList->next;
                   }
                   simulcastList = simulcastList->next;
                 }    
                ///////////////////////////////////////////
                
                pthread_mutex_lock(&mutex);
                numReaders -= 1;
                if(numReaders == 0)
                {
                  pthread_mutex_unlock(&rw_lock);
                }
                pthread_mutex_unlock(&mutex);

                
                 userList = multicastlist;
                 while(userList != NULL) 
                 {    
                   if(client != userList->socket)
                   {
                     send(userList->socket , sbuffer , strlen(sbuffer) , 0 ); 
                   }
                   userList = userList->next;
                 } 
                 multicastlist = NULL;   
            }
           
         memset(buffer, 0, sizeof(1024));
      }
   }

   return NULL;
}
