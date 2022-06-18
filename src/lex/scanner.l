/*	File: scanner.l
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Lexical analyzer for Alpha language
 *		Inserts Token Value to yylval
 *		Returns Token Type to parser
 */


%{
#include "src/lex/lex.h"
#include "parser.h"

%}

%option noyywrap
%option yylineno
%option case-sensitive

IF			"if"
ELSE			"else"
WHILE			"while"
FOR			"for"
FUNC			"function"
RET			"return"
BRK			"break"
CONT			"continue"
AND			"and"|"&&"
NOT			"not"
OR			"or"|"||"
LOC			"local"
TRUE			"true"
FALSE			"false"
NIL			"nil"


ASSIGN			"="
PLUS			"+"
MINUS			"-"
STAR			"*"
SOL			"/"
PER			"%"
EQ			"=="
NEQ			"!="
PLUS2			"++"
MINUS2			"--"
GREAT			">"
LESS			"<"
EGREAT			">="
ELESS			"<="

INT			[0-9]+

REAL			{INT}"."{INT}

LPAR			"("
RPAR			")"
LCBR			"{"
RCBR			"}"
LSBR			"["
RSBR			"]"
SCOL			";"
COMA			","
COL			":"
COL2			"::"
DOT			"."
DOT2			".."

ID			[a-zA-Z][a-zA-Z0-9_]*
STRING			"\""

LCOM			"//".*" "*
MCOM			"/*"

KENO			[ \n\t\\]+

REST			[^(IF-KENO)]

%%
{IF}		{return IF;}
{ELSE}		{return ELSE;}
{WHILE}		{return WHILE;}
{FOR}		{return FOR;}
{FUNC}		{return FUNC;}
{RET}		{return RET;} 
{BRK}		{return BRK;}
{CONT}		{return CONT;}
{AND}		{return AND;}
{NOT}		{return NOT;}
{OR}		{return OR;}
{LOC}		{return LOC;}
{TRUE}		{return TRUE;}
{FALSE}		{return FALSE;}
{NIL}		{return NIL;}

{ASSIGN}	{return ASSIGN;}
{PLUS}		{return PLUS;}
{MINUS}		{return MINUS;}
{STAR}		{return STAR;}
{SOL}		{return SOL;}
{PER}		{return PER;}
{EQ}		{return EQ;}
{NEQ}		{return NEQ;}
{PLUS2}		{return PLUS2;}
{MINUS2}	{return MINUS2;}
{GREAT}		{return GREAT;}
{LESS}		{return LESS;}
{EGREAT}	{return EGREAT;}
{ELESS}		{return ELESS;}
		
{LPAR}		{return LPAR;}
{RPAR}		{return RPAR;}
{LCBR}		{return LCBR;}
{RCBR}		{return RCBR;}
{LSBR}		{return LSBR;}
{RSBR}		{return RSBR;}
{SCOL}		{return SCOL;}
{COMA}		{return COMA;}
{COL}		{return COL;}
{COL2}		{return COL2;}
{DOT}		{return DOT;}
{DOT2}		{return DOT2;}

{REAL}		{yylval.numVal = atof(yytext); return REAL;}	/* char* -> float */
{INT}		{yylval.numVal = atoi(yytext); return INT;}	/* char* -> int */

{ID}		{yylval.strVal = strdup(yytext); return ID; } /* duplicate char* */



{LCOM}		{}

{MCOM}		{	openComm(yylineno);
			char c;
			int errorLine = yylineno;
			while((c=input()) != '\0'){
				if(c == '*'){
					if((c=input()) == '\0'){
						break;
					}else if(c == '/'){
						closeComm(yylineno);
					}else{
						unput(c);
					}
				}else if(c == '/' ){
					if((c=input()) == '\0'){
						break;
					}else if(c == '*'){
						errorLine = yylineno;
						openComm(yylineno);
					}else{
						unput(c);	
					}
				}
				if(commHead->closed){
					break;
				}			
			}
			if(!commHead->closed){
				fprintf(stderr,"ERROR: OPEN COMMENT IN LINE %d\n", errorLine);
				return 0;
			}
		}

{STRING}	{
	
			char *buffer, *final;
			char c;
			int i = 1, line=yylineno;
			
			if ( !(buffer=malloc(sizeof(char)*6000)) ){
				fprintf(stderr,"ERROR: CANT ALLOCATE ENOUGH MEMORY FOR STRING\n");
				return 0;
			}

			buffer[0] = '"';
			while((c=input()) != '\0'){
				if ( c == '\\'){
					if((c = input()) == '\0'){
						unput(c);
						i--;			
						continue;		
					}else if( c == 'n'){
						buffer[i] = '\n';
					}else if( c == 't'){
						buffer[i] = '\t';
					}else if( c == '"'){
						buffer[i] = '\"';
					}else if( c == '\\'){
						buffer[i] = '\\';
					}else{
						buffer[i] = '\\';
						i++;
						buffer[i] = c;
					}
				}else if( c == '"'){
					buffer[i] = '"';
					i++;
					buffer[i] = '\0';
					final = copyStr(buffer);
					yylval.strVal = strdup(final);	/* duplicate char* */
					break;
				}else{
					buffer[i] = c;
				}
				i++;
			}
			if(c == '\0'){
				fprintf(stderr,"ERROR: UNCLOSED STRING IN LINE %d\n", line);
				return 0;
			}
			free(buffer);
		return STRING;
		}


{KENO}		{}

{REST}		{fprintf(stderr,"ERROR: UNDEFINED CHARACTER IN LINE %d\n",yylineno); return 0;}

%%
char* copyStr(char* buffer){
	int i,j;
	char* final;
	
	for(i=1; buffer[i] != '\0'; i++){}
	
	i++;

	if(!(final = malloc(sizeof(char)*i))){
		fprintf(stderr,"ERROR: NOT ENOUGH MEMORY FOR NEW STRING\n");
	}

	for (j=0; j<i; j++){
		final[j] = buffer[j];
	}

	return final;
}


void openComm(unsigned int line) {
    
	struct commNode *temp;

	if(!(temp = malloc(sizeof(struct commNode)))){
		fprintf(stderr, "ERROR: NOT ENOUGH MEMORY FOR NEW COMMENT!\n");
	}

	temp->startline = line;
    	temp->endline = 0;
    	temp->closed = 0;
    	
	if(commHead == NULL) {
		temp->next = NULL;
		temp->prev = NULL;
        	commHead = temp;
        	commTail = temp;
    	}else{
        	temp->prev = commTail;
        	temp->next = NULL;
        	commTail->next = temp;
        	commTail = temp;
	}
}



void closeComm(unsigned int line) {
    
	struct commNode* curr = commTail;

    	while((curr != NULL) && curr->closed ){
        	curr = curr->prev;
    	}
    	curr->closed = 1;
    	curr->endline = line;
}

