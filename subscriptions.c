#include "subscriptions.h"


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


// Funcion para imprimir la lista
void printList(Sub* head) 
{
    Sub* current = head;
    while (current != NULL) 
    {
        printf("mbox: %p, topic: %s\n", current->mbox, current->topic);
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
    sg_mailbox_t* mboxHits = (int*) malloc(sizeof(int) * hits);
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

    // currentizar la cantidad de resultados y devolver el array
    *results = hits;
    return mboxHits;
}
