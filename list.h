#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct USER
{
   char username[30];
   int socket;
   struct ROOM *head; // list of all rooms that user belongs to
   struct USER *dmhead; // list of all users you are connected to
   struct USER *next; //list of all users
};

struct ROOM
{
  char roomname[30];
  struct ROOM *next;
};

/* implement this for USERSINROOM functions */
struct USERSINROOMS
{
  char roomname[30]; // key
  struct USER *head; // value // the linked list of users per room
  struct USERSINROOMS *next;
};

/////////////////// USERLIST FUNCTIONS //////////////////////////

//insert node at the first location
struct USER* insertFirstU(struct USER *head, int socket, char *username);

//find a node with given username
struct USER* findU(struct USER *head, char* username);

//display the list
void printListU(struct USER *node); ///server side
void display_users(struct USER *l, char *buf, int s);

//deletes first item
struct USER* deleteFirstU(struct USER *head);

//is list empty
bool isEmptyU(struct USER *head);


int lengthU(struct USER *head);

//find a link with given socket
struct USER* findSocketU(struct USER *head, int socket);

//extra code I think 
//find a link with given socket
//struct USER* findU(struct USER *head, char* username);

//delete a link with given socket
struct USER* deleteSocketU(struct USER *head, int socket);

//delete a link with given username
struct USER* deleteU(struct USER *head, char* username);
void destroyU(struct USER *head);
//////////////////////////////////////////////////////////////////



/////////////////// ROOMSLIST //////////////////////////
//display the list
void printListRoom(struct ROOM *head);
void display_rooms(struct USERSINROOMS *l, char *buf);
//insert link at the first location
struct ROOM* insertFirstRoom(struct ROOM *head, char *groupname);
//delete first item
struct ROOM* deleteFirstRoom(struct ROOM *head);
//is list empty
bool isEmptyRoom(struct ROOM *head);
int lengthRoom(struct ROOM *head);
//find a link with given socket
struct ROOM* findRoom(struct ROOM *head, char* groupname);
//delete a link with given groupname
struct ROOM* deleteRoom(struct ROOM *head, char* groupname);
void destroyRoom(struct ROOM *head);
/////////////////////////////////////////////////////////////////




/// replace this with up above
/////////////////// USERSINROOMSLIST //////////////////////////
//display the list
void printListUsersInRooms(struct USERSINROOMS *head);
void display_usersinrooms(struct USERSINROOMS *head, char *buf);
//insert link at the first location
struct USERSINROOMS* insertFirstUsersInRooms(struct USERSINROOMS *head,char *groupname); //struct USER *userlinkedlist, char *groupname);
//delete first item
struct USERSINROOMS* deleteFirstUsersInRooms(struct USERSINROOMS *head);
//is list empty
bool isEmptyUsersInRooms(struct USERSINROOMS *head);
int lengthUsersInRooms(struct USERSINROOMS *head);
//find a link with given socket
struct USERSINROOMS* findUsersInRooms(struct USERSINROOMS *head, char* groupname);
//delete a link with given groupname
struct USERSINROOMS* deleteUsersInRooms(struct USERSINROOMS *head, char* groupname);
void destroyUsersInRooms(struct USERSINROOMS *head);
//////////////////////////////////////////////////////////////