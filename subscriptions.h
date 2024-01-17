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


Sub* createSub          (sg_mailbox_t mbox, const char* topic);
void deleteSub          (Sub** head, sg_mailbox_t mbox, const char* topic);
int exists              (Sub* head, sg_mailbox_t mbox, const char* topic) ;
int insertSub           (Sub** head, sg_mailbox_t mbox, const char* topic);
void printList          (Sub* head);
sg_mailbox_t* findSubs  (Sub* head, const char* topic, int* results);