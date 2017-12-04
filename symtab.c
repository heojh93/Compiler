/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

#include "globals.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(Scope *s, char * name, ExpType type, int lineno, int loc )
{ int h = hash(name);
  BucketList l =  s->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->type = type;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = s->hashTable[h];
    s->hashTable[h] = l; }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( Scope *scope, char * name )
{
    if(name == NULL) {
        return NULL;
    }
  int h = hash(name);
  BucketList l =  scope->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return NULL;
  else return l;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing, Scope* scope)
{ 
    treeTraverse(scope);
} /* printSymTab */

void treeTraverse(Scope *scope){
    int i;
    Scope * s = malloc(sizeof(Scope));
    s = scope->child;

    
    fprintf(listing," [Scope : %s]\n", scope->name);
    fprintf(listing,"   Name       Type      Location     Lines\n");
    fprintf(listing,"----------  ----------  -------- ----------------\n");
    
    for (i=0;i<SIZE;++i){ 
        if (scope->hashTable[i] != NULL){ 
            BucketList l = scope->hashTable[i];
            while (l != NULL){ 
                LineList t = l->lines;
                fprintf(listing,"%-12s ",l->name);
                fprintf(listing,"%-13s ",l->type==Integer? "Integer" : "Void");
                fprintf(listing,"%-4d  ",l->memloc);
                while (t != NULL){ 
                    fprintf(listing,"%4d ",t->lineno);
                    t = t->next;
                }
                fprintf(listing,"\n");
                l = l->next;
            }
        }
    }

    fprintf(listing,"\n");
   
    if(scope->child == NULL) return;
    
    else{                
        while(s){
            treeTraverse(s);
            s = s->next;
        }
    }
}

Scope * new_scope(char * name, Scope * parent){
    Scope *newSC = malloc(sizeof(Scope));
    if(newSC){
        newSC->name = name;
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

Scope * get_parent(Scope * s){
    return s->parent;
}

Scope * get_child (Scope * s, char * name){
    Scope *tmp = malloc(sizeof(Scope)); 
    for(tmp = s->child; strcmp(tmp->name,name) != 0; tmp = tmp->next){}
    return tmp;
}

void incr_location (Scope *s){
    s->location += 1;
}

