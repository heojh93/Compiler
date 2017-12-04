/****************************************************/
/* File: cminus.y                                   */
/* The C-MINUS Yacc/Bison specification file        */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#include <stdlib.h>

/* For saving ID names */
typedef struct node{
	char* name;
	int lineno;
	struct node* next;
}saveNode;

saveNode* head = NULL;

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

void pushNode(char *, int);
saveNode popNode();

%}


%token IF ELSE WHILE RETURN INT VOID
%token ID NUM
%token LCURLY RCURLY SEMI

%left COMMA
%right ASSIGN
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left TIMES OVER
%left LPAREN RPAREN LBRACE RBRACE

%token ERROR


%% /* Grammar for C-MINUS */

program				: declaration_list
					    { savedTree = $1; }
					;

declaration_list	: declaration_list declaration
						{ YYSTYPE t = $1;
						  if (t != NULL)
						  { while (t->sibling != NULL)
						    t = t->sibling;
						    t->sibling = $2;
						    $$ = $1; }
						  else $$ = $2;
						}
					| declaration
						{ $$ = $1; }
					;

declaration			: var_declaration
					    { $$ = $1; }
					| fun_declaration
						{ $$ = $1; }
					;

var_declaration		: type_specifier ID	SEMI
                   	 	{	$$ = newExpNode(VarK); 
                   	 		$$->type = (ExpType)$1;

							saveNode node = popNode();
							$$->attr.name = copyString(node.name);
						    $$->lineno = node.lineno;
						} 
                   	 	
					| type_specifier ID LBRACE size RBRACE SEMI
						{	$$ = newExpNode(VarArrK);
							$$->type = (ExpType)$1;
							$$->child[0] = $4;

							saveNode node = popNode();
							$$->attr.name = copyString(node.name);
							$$->lineno = node.lineno;
						}
					;

size				: NUM
						{ $$ = newExpNode(ConstK);
						  $$->type = Integer;
						  $$->attr.val = atoi(tokenString);
						}

type_specifier		: INT
						{	$$ = Integer;	}
					| VOID
						{ $$ = Void; }
					;

fun_declaration		: type_specifier ID	LPAREN params RPAREN compound_stmt
						{	$$ = newStmtNode(FunctionK);
							$$->type = (ExpType)$1;
							$$->child[0] = $4;
							$$->child[1] = $6;

							saveNode node = popNode();
							$$->attr.name = copyString(node.name);
							$$->lineno = node.lineno;
						}
					;

params				: param_list
						{ $$ = $1; }
					| VOID
						{ $$ = newExpNode(SingleParamK);
						  $$->type = Void;
						}
					;

param_list			: param_list COMMA param
						{ YYSTYPE t = $1;
						  if (t != NULL)
						  { while (t->sibling != NULL)
						    t = t->sibling;
						    t->sibling = $3;
						    $$ = $1; }
						  else $$ = $3;
						}
					| param
						{ $$ = $1; }
					;

param				: type_specifier ID
					    { $$ = newExpNode(SingleParamK);
						  $$->type = (ExpType)$1;	
												
						  saveNode node = popNode();
						  $$->attr.name = copyString(node.name);
						  $$->lineno = node.lineno;
                   		}
					| type_specifier ID LBRACE RBRACE
						{ $$ = newExpNode(ArrParamK);
						  $$->type = (ExpType)$1;
												
						  saveNode node = popNode();
						  $$->attr.name = copyString(node.name);
						  $$->lineno = node.lineno;

						}
					;

compound_stmt		: LCURLY local_declarations statement_list RCURLY
						{ $$ = newStmtNode(CompoundK);
						  $$->child[0] = $2;
						  $$->child[1] = $3;											
						}
					;

local_declarations	: local_declarations var_declaration
						{ YYSTYPE t = $1;
						  if (t != NULL)
						  { while (t->sibling != NULL)
						    t = t->sibling;
						    t->sibling = $2;
						    $$ = $1; }
						  else $$ = $2;
						}
					| empty
						{ $$ = $1; }
					;

statement_list		: statement_list statement
						{ YYSTYPE t = $1;
						  if (t != NULL)
						  { while (t->sibling != NULL)
						    t = t->sibling;
						    t->sibling = $2;
						    $$ = $1; }
						    else $$ = $2;
						}
					| empty
						{ $$ = $1; }
					;

statement			: expression_stmt
						{ $$ = $1; }
					| compound_stmt
						{ $$ = $1; }
					| selection_stmt
						{ $$ = $1; }
					| iteration_stmt
						{ $$ = $1; }
					| return_stmt
						{ $$ = $1; }
					;

expression_stmt		: expression SEMI
						{ $$ = $1; }
					| SEMI
						{ $$ = NULL; }
					;

selection_stmt		: IF LPAREN expression RPAREN statement
						{ $$ = newStmtNode(IfK);
						  $$->attr.elseFlag = 0;
						  $$->child[0] = $3;
						  $$->child[1] = $5;									
                        }
					| IF LPAREN expression RPAREN statement ELSE statement
						{	$$ = newStmtNode(IfK);
							$$->attr.elseFlag = 1;
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
						}
					;

iteration_stmt		: WHILE LPAREN expression RPAREN statement
						{	$$ = newStmtNode(WhileK);
							$$->child[0] = $3;
							$$->child[1] = $5;
						}
					;

return_stmt			: RETURN SEMI
						{ $$ = newStmtNode(ReturnK); }
					| RETURN expression SEMI
						{ $$ = newStmtNode(ReturnK);
						  $$->child[0] = $2;
						}
					;

expression			: var ASSIGN expression
						{ $$ = newExpNode(AssignK);
						  $$->child[0] = $1;
						  $$->child[1] = $3;
						}
					| simple_expression
						{ $$ = $1; }
					;

var					: ID
						{
						    $$ = newExpNode(IdK);
												
							saveNode node = popNode();
							$$->attr.name = copyString(node.name);
							$$->lineno = node.lineno;

						}
					| ID LBRACE expression RBRACE
						{ $$ = newExpNode(IdK);
						  $$->child[0] = $3;
											
						  saveNode node = popNode();
						  $$->attr.name = copyString(node.name);
						  $$->lineno = node.lineno;
						}
					;

simple_expression	: additive_expression relop additive_expression
						{ $$ = newExpNode(OpK);
						  $$->attr.op = $2;
						  $$->child[0] = $1;
						  $$->child[1] = $3;								
						}
					| additive_expression
						{ $$ = $1; }
					;

relop				: LE
						{ $$ = LE; }
					| LT
						{ $$ = LT; }
					| GT
						{ $$ = GT; }
					| GE
						{ $$ = GE; }
					| EQ
						{ $$ = EQ; }
					| NE
					    { $$ = NE; }
					;

additive_expression	: additive_expression addop term
						{ $$ = newExpNode(OpK);
						  $$->attr.op = $2;
						  $$->child[0] = $1;
					      $$->child[1] = $3;
						}
					| term
						{ $$ = $1; }
					;

addop				: PLUS
						{ $$ = PLUS; }
					| MINUS
						{ $$ = MINUS; }
					;

term				: term mulop factor
						{ $$ = newExpNode(OpK);
						  $$->attr.op = $2;
						  $$->child[0] = $1;
						  $$->child[1] = $3;
						}
					| factor
						{ $$ = $1; }
					;

mulop				: TIMES
						{ $$ = TIMES; }
					| OVER
						{ $$ = OVER; }
					;

factor				: LPAREN expression RPAREN
						{ $$ = $2; }
					| var
						{ $$ = $1; }
					| call
						{ $$ = $1; }
					| NUM
						{ $$ = newExpNode(ConstK);
					      $$->type = Integer;
                  		  $$->attr.val = atoi(tokenString);
                 		}
					;

call				: ID LPAREN args RPAREN
						{ $$ = newExpNode(CallK);
						  $$->child[0] = $3;
												
						  saveNode node = popNode();
						  $$->attr.name = copyString(node.name);
						  $$->lineno = node.lineno;

                 		}
					;

args				: arg_list
						{ $$ = $1; }
					| empty
						{ $$ = $1; }
					;

arg_list			: arg_list COMMA expression
						{ YYSTYPE t = $1;
					      if (t != NULL)
					      { while (t->sibling != NULL)
					        t = t->sibling;
					        t->sibling = $3;
					        $$ = $1; }
					      else $$ = $3;
						}
					| expression
						{ $$ = $1; }
					;

empty				: { $$ = NULL; }
					;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ 
	TokenType token = getToken();
	if(token == ID){
		pushNode(tokenString,lineno);
	}
	
	return token; }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

void pushNode(char* token, int lineno){
	
	saveNode* newNode = malloc(sizeof(saveNode));

	newNode->name = copyString(token);
	newNode->lineno = lineno;

	newNode->next = head;
	head = newNode;
}

saveNode popNode(){

	saveNode* nextNode = NULL;
	saveNode tmp;

	nextNode = head->next;
	
	tmp.name = copyString(head->name);
	tmp.lineno = head->lineno;

	free(head);
	head = nextNode;

	return tmp;
}
