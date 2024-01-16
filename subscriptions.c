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

        if (*head == NULL) {
            *head = newSub;
        } else {
            Sub* current = *head;
            while (current->next != NULL) {
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


// Función para imprimir la lista
void printList(Sub* head) 
{
    Sub* current = head;
    while (current != NULL) 
    {
        printf("mbox: %p, topic: %s\n", current->mbox, current->topic);
        current = current->next;
    }
}

/*


// Función principal
int main() 
{
    Sub* lista = NULL;

    // Insertar Subs en la lista
    insertarSub(&lista, 42, "Hola");
    insertarSub(&lista, 17, "Mundo");
    insertarSub(&lista, 42, "Programación");
    insertarSub(&lista, 10, "C");

    // Imprimir la lista
    printf("Lista original:\n");
    imprimirLista(lista);

    // Eliminar un Sub por el parámetro topic
    const char* topicAEliminar = "Mundo";
    eliminarSubPortopic(&lista, topicAEliminar);

    // Imprimir la lista después de la eliminación
    printf("\nLista después de eliminar el Sub con topic '%s':\n", topicAEliminar);
    imprimirLista(lista);

    // Liberar memoria al finalizar
    Sub* current = lista;
    while (current != NULL) {
        Sub* next = current->next;
        free(current);
        current = next;
    }

    return 0;
}*/