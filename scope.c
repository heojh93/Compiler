#include "scope.h"
#include <string.h>

Scope * new_scope(char * name, Scope * parent){
    Scope *newSC = malloc(sizeof(Scope));
    if(newSC){
        strcpy(newSC->name, name);
        newSC->location = 0;
        newSC->isVisited = 0;
        memset(newSC->hashTable, 0, sizeof(newSC->hashTable));
        newSC->child = NULL;
        newSC->parent = parent;
    }
    return newSC;
}

Scope * add_sibling (Scope * s, char * name, Scope * parent){
    if(s == NULL) return NULL;

    while(s->next) s = s->next;
    
    return (s->next = new_scope(name, parent));
}

Scope * add_child (Scope * s, char * name){
    if(s == NULL) return NULL;

    if(s->child) 
        return add_sibling(s->child, name, s);
    else 
        return (s->child = new_scope(name, s));
}

void incr_location (Scope *s){
    s->location += 1;
}
