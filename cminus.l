/****************************************************/
/* File: cminus.l                                   */
/* Lex specification for CMINUS                     */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"					{return WHILE;}
"return"				{return RETURN;}
"int"						{return INT;}
"void"					{return VOID;}

"=="            {return EQ;}
"!="						{return NE;}
"="             {return ASSIGN;}
"<"             {return LT;}
"<="						{return LE;}
">"							{return GT;}
">="						{return GE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"{"							{return LCURLY;}
"}"							{return RCURLY;}
"["							{return LBRACE;}
"]"							{return RBRACE;}
";"             {return SEMI;}
","							{return COMMA;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            { char c;
									int chk = 0;
									int findEnd = 0;
                  do
                  { c = input();
										if(chk){
											if(c=='/') findEnd = 1;
											else chk = 0;
										}
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
										if (c == '*') chk = 1;
                  } while (!findEnd);
                }
"/"							{return OVER;}
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

