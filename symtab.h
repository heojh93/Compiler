/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

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


/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( Scope* s, char * name, ExpType type, int lineno, int loc );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
// int st_lookup ( char * name );
BucketList st_lookup ( Scope * scope, char * name);
BucketList st_lookup_excluding_parent ( char * scope, char * name);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing, Scope* scope);
void treeTraverse(Scope * scope);

#endif
