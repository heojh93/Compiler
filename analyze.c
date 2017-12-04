/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
// static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ 
  Scope *s = malloc(sizeof(Scope));
  s = Cur_Scope;

  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case FunctionK:
            // Parent must be Global
            if(s->parent != NULL){
                fprintf(listing, "Can't Define Function in this Scope!\n");
            }
            // Insert into Sym Table
            else if(!strcmp(t->attr.name, "input") || !strcmp(t->attr.name, "output")){
                st_insert(Head_Scope, t->attr.name, t->type, t->lineno, Head_Scope->location++);
            }
            // Already Exist
            else if (st_lookup(s, t->attr.name) != NULL){
                fprintf(listing, "Function %s Already Exists!\n", t->attr.name);
            }

            else{
                Cur_Scope = add_child(Cur_Scope, t->attr.name);
                st_insert(s, t->attr.name, t->type, t->lineno, s->location++);
            }
            break;
        default:
            break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { 
        case VarArrK:
        case VarK:
        case SingleParamK:
        case ArrParamK:
            // Not Found
            //printf("%s\n", t->attr.name);
            if (t->attr.name != NULL & st_lookup(s, t->attr.name) == NULL) {
                st_insert(s, t->attr.name, t->type, t->lineno, s->location++);
            }
            
            break;
        case IdK:
            // Found 
            if(t->attr.name != NULL & st_lookup(s, t->attr.name) != NULL)
                st_insert(s, t->attr.name, t->type, t->lineno, 0);

            break;

        case CallK:
            if(st_lookup(Head_Scope, t->attr.name) == NULL)
                fprintf(listing, "Undeclared Function\n");
            else
                st_insert(Head_Scope, t->attr.name, t->type, t->lineno, 0);
            break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


void popAfterInsert(TreeNode * t){
    if(t->nodekind == StmtK && t->kind.stmt == CompoundK){
        Cur_Scope = get_parent(Cur_Scope);
    }
}

void IOFunc(TreeNode ** t){
    TreeNode *input;
    TreeNode *output;
    TreeNode *param;

    input = newStmtNode(FunctionK);
    input->attr.name = copyString("input");
    input->type = Integer;
    input->lineno = 0;
    input->sibling = *t;
    *t = input;

    param = newStmtNode(ParamK);
    param->attr.name = copyString("arg");
    param->type = Integer;
    param->lineno = 0;

    output = newStmtNode(FunctionK);
    output->attr.name = copyString("output");
    output->type = Void;
    output->lineno = 0;
    output->child[0] = param;
    output->sibling = *t;
    *t = output;
}


/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{
    Head_Scope = new_scope("Global", NULL);
  
    IOFunc(&syntaxTree);

    Cur_Scope = malloc(sizeof(Scope)); 
    Cur_Scope = Head_Scope;
    
    traverse(syntaxTree,insertNode, popAfterInsert);
    if (TraceAnalyze){ 
        fprintf(listing,"\nSymbol table:\n\n");
        printSymTab(listing, Head_Scope);
    }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ 
  Scope * s = Cur_Scope;
  BucketList bucket;
  ExpType type;
  ExpKind e;
  ExpKind e2;
 // printf("attr name : %s\n", t->attr.name);

  switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { 
        case OpK:
            break;
        case ConstK:
            t->type = Integer;
            break;
        case SingleParamK:
        case ArrParamK:
            break;
        case IdK:
            bucket = st_lookup(s, t->attr.name);
            if(bucket == NULL){
                typeError(t, "Not defined Variable");
                fprintf(listing, "   --> %s ", t->attr.name);
                fprintf(listing, "in [%s]\n", s->name);
            }
            else if(bucket->type != Integer){
                typeError(t, "Variable Type must be Integer");
                fprintf(listing, "   --> %s\n", t->attr.name);
            }
            break;
        case VarK:
        case VarArrK:
            if(t->type != Integer){
                typeError(t, "Variable Must be Integer Type!");
                fprintf(listing, "   --> %s\n", t->attr.name);
            }
            break;
        case AssignK:

            e = t->child[1]->kind.exp;
            if(e == IdK){
                type = st_lookup(s, t->child[1]->attr.name)->type;
            }
            else if(e == CallK){ 
                type = st_lookup(Head_Scope, t->child[1]->attr.name)->type;
            }
            else if(e == VarK || e == VarArrK ){
                type = t->child[1]->type;
            }
            else {
                type = t->type;
            }
            if(type != Integer){
                typeError(t, "Type Error while Assign!\n");
            }

            break;
        case CallK:
            break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { 
        case IfK:
            break;
        case WhileK:
            break;
        case FunctionK:
            type = t->type;
            if(!(type==Integer || type==Void)){
                typeError(t, "Function type must be Integer or Void\n");
            }
            break;
        case CompoundK:
            Cur_Scope = get_parent(Cur_Scope);
            break;
        case ParamK:
            break;
        
        case ReturnK:
            type = t->type;
            if(t->type == Void){
                // return x;
                if(t->child[0]){
                    if(t->child[0]->type != type){
                        typeError(t, "Funcion Type != Return Type\n");
                    }
                }
            }
            else if(t->type == Integer){
                // return x;
                if(t->child[0]){
                    if(t->child[0]->type != type){
                        typeError(t, "Function Type != Return Type\n");
                    }
                }
                else{
                    typeError(t, "Must Return Integer Type!\n");
                }
            }
            break;

        default:
          break;
      }
      break;
    default:
      break;

  }
}

void pushBeforeCheck (TreeNode* t){
    if(t->nodekind == StmtK && t->kind.stmt == FunctionK){
        Cur_Scope = get_child(Cur_Scope, t->attr.name);
    }
}


/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree, pushBeforeCheck, checkNode);
}
