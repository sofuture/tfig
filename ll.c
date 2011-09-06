#include <stdio.h>
#include <stdlib.h>

#include "ll.h"

node *createnode(double x, double y){
    node *newNode = malloc(sizeof(node));
    newNode->x = x;
    newNode->y = y;
    newNode->next = NULL;
    return newNode;
}

list *emptylist(){
    list *list = malloc(sizeof(list));
    list->head = NULL;
    list->ct = 0;
    return list;
}

void add(double x, double y, list *list){
    node *current = NULL;

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

void destroy(list * list){
    node *current = list->head;
    node *next = current;
    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
