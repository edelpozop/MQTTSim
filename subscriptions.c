#include "subscriptions.h"

// Lista enlazada global
Sub* globalSubs = NULL;
pthread_mutex_t mutex;

void init_subscriptions()
{
    if (pthread_mutex_init(&mutex, NULL) != 0) 
    {
        perror("Error mutex");
        exit(EXIT_FAILURE);
    }
}

void end_subscriptions()
{
    // Liberar memoria de la lista global y de las listas locales
    Sub* currentList = globalSubs;
    while (currentList != NULL) 
    {
        Sub* nextList = currentList->next;
        freeLocalList(currentList);
        currentList = nextList;
    }
    pthread_mutex_destroy(&mutex);
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


int insertSub(Sub** head, sg_mailbox_t mbox, const char* topic)
{
    if (!exists(*head, mbox, topic)) 
    {
        Sub* newSub = createSub(mbox, topic);

        if (*head == NULL) 
        {
            *head = newSub;
        } else {
            Sub* current = *head;
            while (current->next != NULL) 
            {
                current = current->next;
            }
            current->next = newSub;
        }
        return 0;
    }
    else return 1;
}



void deleteSub(Sub** head, sg_mailbox_t mbox, const char* topic) 
{
    Sub* current = *head;
    Sub* prev = NULL;

    while (current != NULL) 
    {
        if (current->mbox == mbox && strcmp(current->topic, topic) == 0) 
        {
            if (prev == NULL) 
            {
                // El Sub a eliminar es el primero
                *head = current->next;
            } else 
            {
                // El Sub a eliminar no es el primero
                prev->next = current->next;
            }

            // Liberar memoria del Sub eliminado
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }
}


// Funcion para buscar mboxs por topic y devolver una lista de mboxs coincidentes
sg_mailbox_t* findSubs(Sub* head, const char* topic, int* results) 
{
    // Contar la cantidad de mboxs que coinciden
    int hits = 0;
    Sub* current = head;
    while (current != NULL) 
    {
        if (strcmp(current->topic, topic) == 0) 
        {
            hits++;
        }
        current = current->next;
    }

    // Crear un array dinámico para almacenar los mbox coincidentes
    sg_mailbox_t* mboxHits = (sg_mailbox_t*) malloc(sizeof(sg_mailbox_t) * hits);
    if (mboxHits == NULL) 
    {
        perror("Error al asignar memoria para el array de mbox coincidentes");
        exit(EXIT_FAILURE);
    }

    // Almacenar los mboxs coincidentes en el array
    current = head;
    int index = 0;
    while (current != NULL) 
    {
        if (strcmp(current->topic, topic) == 0) 
        {
            mboxHits[index] = current->mbox;
            index++;
        }
        current = current->next;
    }

    printf("%s %d\n", topic, hits);

    *results = hits;
    return mboxHits;
}


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


// Función para insertar una lista local en la lista global
void insertGlobalList(Sub* head) 
{
    pthread_mutex_lock(&mutex);
    if (globalSubs == NULL) 
    {
        globalSubs = head;
    } 
    else 
    {
        Sub* current = globalSubs;
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = head;
    }
    pthread_mutex_unlock(&mutex);
}

// Funcion para imprimir la lista
void printLocalList(Sub* head) 
{
    Sub* current = head;
    while (current != NULL) 
    {
        printf("mbox: %p, topic: %s\n", current->mbox, current->topic);
        current = current->next;
    }
    printf("\n");
}


// Función para imprimir la lista global
void printGlobalList() 
{
    pthread_mutex_lock(&mutex);
    Sub* currentList = globalSubs;
    while (currentList != NULL) 
    {
        printLocalList(currentList);
        currentList = currentList->next;
    }
    pthread_mutex_unlock(&mutex);
}


// Función para liberar la memoria de una lista local
void freeLocalList(Sub* head) 
{
    Sub* current = head;
    while (current != NULL) 
    {
        Sub* next = current->next;
        free(current);
        current = next;
    }
}