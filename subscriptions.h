#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <simgrid/mailbox.h>

// Definición de la estructura del nodo
typedef struct Sub 
{
    sg_mailbox_t mbox;
    char topic[128];
    struct Sub* next;
} Sub;

// Lista enlazada
typedef struct 
{
    Sub* head;
} Subscriptions;

typedef struct 
{
    sg_mailbox_t* mailboxes;
    int count;
} MailboxList;

void init_subscriptions     ();
void end_subscriptions      ();
//Sub* createSub              (sg_mailbox_t mbox, const char* topic);
void deleteSub              (sg_mailbox_t mbox, const char* topic);
int exists                  (Sub* head, sg_mailbox_t mbox, const char* topic) ;
int insertSub               (sg_mailbox_t mbox, const char* topic);
MailboxList findSubs        (const char* topic);
sg_mailbox_t* findAllSubs   (const char* topic, int* results) ;
void printList              ();
void freeList               ();