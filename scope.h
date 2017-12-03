#ifndef _SCOPE_H_
#define _SCOPE_H_

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

#define SIZE 211

typedef struct LineListRec
{
    int lineno;
    struct LineListRec * next;
} *LineList;

typedef struct BucketListRec
{
    char * name;
    ExpType type;
    LineList lines;
    int memloc;
    struct BucketListRec * next;
} * BucketList;


typedef struct node
{
    char * name;
    int location;
    int isVisited;
    BucketList hashTable[SIZE];
    struct node * next;
    struct node * child;
    struct node * parent;
}Scope;

Scope * new_scope (char * name, Scope * parent);
Scope * add_sibling (Scope * s, char * name, Scope * parent);
Scope * add_child (Scope * s, char * name);
Scope * get_parent (Scope * s);
void incr_location (Scope * s);

static Scope * Head_Scope;
static Scope * Cur_Scope;


#endif
