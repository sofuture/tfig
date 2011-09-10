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

list *ll_new(){
    list *list = malloc(sizeof(list));
    list->head = list->tail = NULL;
    list->ct = 0;
    return list;
}

void ll_add(double x, double y, list *list){
    if(list->head == NULL){
        list->head = list->tail = createnode(x, y);
    } else {
        list->tail = list->tail->next = createnode(x, y);
    }
    list->ct++;
}

void ll_free(list * list){
    node *current = list->head;
    node *next = current;
    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
