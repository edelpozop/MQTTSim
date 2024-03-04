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


void init_subscriptions     ();
void end_subscriptions      ();
Sub* createSub              (sg_mailbox_t mbox, const char* topic);
void deleteSub              (Sub** head, sg_mailbox_t mbox, const char* topic);
int exists                  (Sub* head, sg_mailbox_t mbox, const char* topic) ;
int insertSub               (Sub** head, sg_mailbox_t mbox, const char* topic);
sg_mailbox_t* findSubs      (Sub* head, const char* topic, int* results);
sg_mailbox_t* findAllSubs   (const char* topic, int* results) ;
void printLocalList         (Sub* head);
void printGlobalList        ();
void insertGlobalList       (Sub* head);
void freeLocalList          (Sub* head);