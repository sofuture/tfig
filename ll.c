#include "ll.h"

Node *createnode(int x, int y){
    Node *newNode = malloc(sizeof(Node));
    newNode->x = x;
    newNode->y = y;
    newNode->next = NULL;
    return newNode;
}

List *emptylist(){
    List *list = malloc(sizeof(List));
    list->head = NULL;
    list->ct = 0;
    return list;
}

void add(int x, int y, List *list){
    Node *current = NULL;

    if(list->head == NULL){
        list->head = createnode(x, y);
    } else {
        current = list->head; 
        while (current->next!=NULL){
            current = current->next;
        }
        current->next = createnode(x, y);
    }
    list->ct++;
}

void destroy(List * list){
    Node *current = list->head;
    Node *next = current;
    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
