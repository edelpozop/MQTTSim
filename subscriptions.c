#include "subscriptions.h"

// Lista enlazada global
//pthread_mutex_t mutex;
Subscriptions globalSubs;

void init_subscriptions()
{
    globalSubs.head = NULL;
    /*if (pthread_mutex_init(&mutex, NULL) != 0) 
    {
        perror("Error mutex");
        exit(EXIT_FAILURE);
    }*/
}

void end_subscriptions()
{
    // Liberar memoria de la lista global y de las listas locales
    freeList();
    //pthread_mutex_destroy(&mutex);
}


// Función para crear un nuevo Sub
Sub* createSub(sg_mailbox_t mbox, const char* topic)
{
    Sub* newSub = (Sub*)malloc(sizeof(Sub));
    if (newSub == NULL) 
    {
        perror("Error al asignar memoria para el nuevo Sub");
        exit(EXIT_FAILURE);
    }

    newSub->mbox = mbox;
    strncpy(newSub->topic, topic, sizeof(newSub->topic) - 1);
    newSub->topic[sizeof(newSub->topic) - 1] = '\0';  // Asegurar que el array de topic este terminado con '\0'
    newSub->next = NULL;

    return newSub;
}



int exists(Sub* head, sg_mailbox_t mbox, const char* topic) 
{
    Sub* current = head;
    while (current != NULL) 
    {
        if (current->mbox == mbox && strcmp(current->topic, topic) == 0) 
        {
            return 1; 
        }
        current = current->next;
    }
    return 0; 
}


int insertSub(sg_mailbox_t mbox, const char* topic)
{
    Sub* newSub = (Sub*) malloc(sizeof(Sub));
    if (newSub == NULL) 
    {
        perror("Error al asignar memoria para el nuevo nodo");
        return 1;
    }

    newSub->mbox = mbox;
    strncpy(newSub->topic, topic, sizeof(newSub->topic) - 1);
    newSub->topic[sizeof(newSub->topic) - 1] = '\0';  // Asegurar que el array de topic este terminado con '\0'
    newSub->next = NULL;

    if (globalSubs.head == NULL) 
    {
        globalSubs.head = newSub;
    } 
    else 
    {
        Sub* current = globalSubs.head;
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = newSub;
    }
    return 0;
}



void deleteSub(sg_mailbox_t mbox, const char* topic) 
{
    Sub* current = globalSubs.head;
    Sub* prev = NULL;

    while (current != NULL) 
    {
        if (strcmp(current->topic, topic) == 0 && memcmp(&current->mbox, &mbox, sizeof(sg_mailbox_t)) == 0) 
        {
            if (prev == NULL) 
            {
                globalSubs.head = current->next;
            } 
            else 
            {
                prev->next = current->next;
            }

            free(current->topic);
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }

}


// Funcion para buscar mboxs por topic y devolver una lista de mboxs coincidentes
MailboxList findSubs(const char* topic) 
{
    MailboxList result;
    result.mailboxes = NULL;
    result.count = 0;

    Sub* current = globalSubs.head;

    while (current != NULL) {
        if (strcmp(current->topic, topic) == 0) 
        {
            result.count++;
            result.mailboxes = realloc(result.mailboxes, result.count * sizeof(sg_mailbox_t));
            if (result.mailboxes == NULL) 
            {
                perror("Error al asignar memoria para la lista de mailboxes");
                exit(EXIT_FAILURE);
            }

            result.mailboxes[result.count - 1] = current->mbox;
        }
        current = current->next;
    }

    return result;
}

/*
// Funcion para buscar mboxs por topic y devolver una lista de mboxs coincidentes
sg_mailbox_t* findAllSubs(const char* topic, int* results) 
{
    pthread_mutex_lock(&mutex);  // Bloquear el mutex antes de acceder a la lista global

    // Contar la cantidad de enteros que coinciden
    int resultsT = 0;
    Sub* currentList = globalSubs;
    while (currentList != NULL) 
    {
        int localResult = 0;
        sg_mailbox_t* mailboxHitLocal = findSubs(currentList, topic, &localResult);
        resultsT += localResult;
        free(mailboxHitLocal);
        currentList = currentList->next;
    }

    // Crear un array dinámico para almacenar los enteros coincidentes
    sg_mailbox_t* totalMailbox = (sg_mailbox_t*) malloc(sizeof(sg_mailbox_t) * resultsT);
    if (totalMailbox == NULL) 
    {
        perror("Error al asignar memoria para el array de enteros coincidentes");
        exit(EXIT_FAILURE);
    }

    // Almacenar los enteros coincidentes en el array
    int index = 0;
    currentList = globalSubs;
    while (currentList != NULL) 
    {
        int localResult = 0;
        sg_mailbox_t* mailboxHitLocal = findSubs(currentList, topic, &localResult);
        for (int i = 0; i < localResult; i++) 
        {
            totalMailbox[index] = mailboxHitLocal[i];
            index++;
        }
        free(mailboxHitLocal);
        currentList = currentList->next;
    }

    pthread_mutex_unlock(&mutex);  // Desbloquear el mutex después de acceder a la lista global

    // Actualizar la cantidad de resultados y devolver el array
    *results = resultsT;
    return totalMailbox;
}
*/



void freeList() 
{
    Sub* current = globalSubs.head;
    Sub* next;

    while (current != NULL) 
    {
        next = current->next;
        //free(current->topic);
        free(current);
        current = next;
    }

    globalSubs.head = NULL;
}