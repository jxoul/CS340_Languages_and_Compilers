/*	File: parser.y
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Syntax analyzer for Alpha language
 *		Creates Symbol Table
 *		Prints Symbol Table 
 *		Prints Grammar Rules
 */

%{
	#include "src/funcs/funcs.h"
	#include "src/manager/manager.h"
	#include "src/tcode/tcode.h"
%}
/*
*	Dilwseis
*/

/* expect 1 confict ifelse */

/* starting point Rule */
%start program 



/* available Types */
%union{
	unsigned int uintVal;				/* unsigned int */
	char* strVal;					/* strings characters */
	double numVal;					/* numebers */
	struct SymbolTableEntry* symVal;		/* symbols */
	struct Expression* exprVal;			/* expressions */
	struct Call* callVal;				/* function calls */
	struct Indexed* indexVal;			/* indexed */
	struct Statement* stmtVal;			/* statements */
	struct Forprefix* forVal;			/* forprefix */
}

/* no Type Tokens */
%token IF ELSE WHILE FOR FUNC RET BRK CONT AND NOT OR LOC TRUE FALSE NIL ASSIGN PLUS MINUS STAR SOL PER EQ NEQ PLUS2 MINUS2 GREAT LESS EGREAT ELESS LPAR RPAR LCBR RCBR LSBR RSBR SCOL COMA COL COL2 DOT DOT2 UMINUS

/* Type Tokens */
%token 	<numVal> 	INT REAL
%token 	<strVal> 	ID STRING


/* Type Rules */
%type <exprVal>		lvalue member expr exprs primary assignexpr const elist call objectdef_opts objectdef term ifprefix whilecond
%type <strVal>		funcname
%type <symVal>		funcprefix funcdef
%type <uintVal>		whilestart  N M
%type <callVal>		callsuffix normcall methodcall
%type <indexVal>	indexed indexedelem indexedelems
%type <stmtVal>		stmt stmts loopstmt block if returnstmt funcbody
%type <forVal>		forprefix

/* proteraiotita & proseteristikotita */
%right		ASSIGN
%left		OR
%left		AND
%nonassoc	EQ NEQ
%nonassoc	GREAT EGREAT LESS ELESS
%left		PLUS MINUS
%left		STAR SOL PER
%right		NOT PLUS2 MINUS2 UMINUS
%left 		DOT DOT2
%left		LSBR RSBR
%left		LPAR RPAR

%%
/*
*	Perigrafi
*/

program:	 stmts
	|	/* exmpty */
	;

stmts:		stmt 			{$$ = Manager_stmt($1);}
     	|	stmts stmt		{$$ = Manager_stmts($1,$2);}
	;

stmt:		expr SCOL 		{$$ = Manager_stmt_expr($1);} 
    	|	if  			{$$ = Manager_stmt_stmt($1);}
	|	while 			{$$ = Manager_stmt_null(); }
	|	for			{$$ = Manager_stmt_null();}
	|	returnstmt 		{$$ = Manager_stmt_stmt($1);}
	|	BRK SCOL 		{$$ = Manager_break();}
	|	CONT SCOL		{$$ = Manager_continue();}
	|	block 			{$$ = Manager_stmt_stmt($1);}
	|	funcdef 		{$$ = Manager_stmt_null();}
	|	SCOL 			{$$ = Manager_stmt_null();}
	;


expr:		assignexpr		{$$ = Manager_expr_assignexpr($1);}
    	|	expr PLUS expr		{$$ = Manager_expr_arithop($1,add,$3);} 	
	|	expr MINUS expr		{$$ = Manager_expr_arithop($1,sub,$3);}
	|	expr STAR expr		{$$ = Manager_expr_arithop($1,mul,$3);}
	|	expr SOL expr		{$$ = Manager_expr_arithop($1,Div,$3);}
	|	expr PER expr		{$$ = Manager_expr_arithop($1,mod,$3);}
    	|	expr GREAT expr 	{$$ = Manager_expr_relop($1,if_greater,$3);}
    	|	expr EGREAT expr	{$$ = Manager_expr_relop($1,if_greatereq,$3);}
    	|	expr LESS expr 		{$$ = Manager_expr_relop($1,if_less,$3);}
    	|	expr ELESS expr		{$$ = Manager_expr_relop($1,if_lesseq,$3);}
    	|	expr EQ expr 		{$$ = Manager_expr_relop($1,if_eq,$3);}
    	|	expr NEQ expr 		{$$ = Manager_expr_relop($1,if_noteq,$3);}
    	|	expr AND		{Manage_check_expr($1);} 
		M expr 			{$$ = Manager_expr_boolop($1,and,$4,$5);}
    	|	expr OR 		{Manage_check_expr($1);}
		M expr 			{$$ = Manager_expr_boolop($1,or,$4,$5);}
	|	term			{$$ = Manager_expr_term($1);} 
	;



term:		LPAR expr RPAR 		{$$ = Manager_term_expr($2);} 
    	|	MINUS expr %prec UMINUS {$$ = Manager_term_minusexpr($2);}
	|	NOT expr 		{$$ = Manager_term_notexpr($2);}
	|	PLUS2 lvalue 		{$$ = Manager_term_plus2lvalue($2);} 
	|	lvalue PLUS2 		{$$ = Manager_term_lvalueplus2($1);}
	|	MINUS2 lvalue 		{$$ = Manager_term_minus2lvalue($2);}
	|	lvalue MINUS2 		{$$ = Manager_term_lvalueminus2($1);}
	|	primary			{$$ = Manager_term_primary($1); }
	;

assignexpr:	lvalue 	ASSIGN expr 	{$$ = Manager_assignexpr($1,$3);} 
	;

primary:	lvalue 			{$$ = Manager_primary_lvalue($1);}
       	|	call 			{$$ = Manager_primary_call($1); }
	|	objectdef 		{$$ = Manager_primary_objectdef($1);}
	|	LPAR funcdef RPAR 	{$$ = Manager_primary_funcdef($2);} 
	|	const 			{$$ = Manager_primary_const($1); }
	;

lvalue:		ID			{$$ = Manager_lvalue_id($1);}
     	|	LOC ID 			{$$ = Manager_lvalue_localid($2);}
	|	COL2 ID			{$$ = Manager_lvalue_col2id($2);}
	|	member 			{$$ = Manager_lvalue_member($1);}
	;


member:		lvalue DOT ID 		{$$ = Manager_member_lvalueid($1,$3);} 
      	|	lvalue LSBR expr RSBR 	{$$ = Manager_member_lvalueexpr($1,$3);}
	|	call DOT ID 		{$$ = Manager_member_callid($1,$3);}
	|	call LSBR expr RSBR  	{$$ = Manager_member_callexpr($1,$3); }
	;

call:		call LPAR elist RPAR 	{$$ = Manager_call_callelist($1,$3);} 
	|	lvalue callsuffix 	{$$ = Manager_call_lvaluecallsuffix($1,$2);}
	|	LPAR funcdef RPAR LPAR elist RPAR { $$ = Manager_call_funcdefelist($2,$5);}
	;

callsuffix:	normcall 		{$$ = Manager_callsuffix_normcall($1); }
	|	methodcall 		{$$ = Manager_callsuffix_methodcall($1); }
	;

normcall:	LPAR elist RPAR 	{$$ = Manager_normcall($2); }
	;
	
methodcall:	DOT2 ID LPAR elist RPAR {$$ = Manager_methodcall($2,$4);}
	;

elist:		exprs 			{$$ = Manager_elist_exprs($1);}
   	|				{$$ = Manager_elist_empty();} 
	;

exprs:		expr			{$$ = Manager_exprs_expr($1);}
     	|	exprs COMA expr		{$$ = Manager_exprs_exprs($1,$3); } 
	;

objectdef:	LSBR objectdef_opts RSBR{$$ = Manager_objectdef($2);} 
	;

objectdef_opts:		elist 		{$$ = Manager_objectdef_elist($1);} 
	|		indexed 	{$$ = Manager_objectdef_indexed($1);}
	;

indexed:	indexedelems 		{$$ = Manager_indexed($1);}
	;

indexedelems:	indexedelem 		{$$ = Manager_indexed_element($1);}
	|	indexedelems COMA indexedelem {$$ = Manager_indexed_elements($1,$3);}
	;	

indexedelem:	LCBR expr COL expr RCBR {$$ = Manager_indexedelem($2,$4);}
	;

block:		LCBR 			{Manager_block_open();} 
     		RCBR 			{$$ = Manager_block_close(NULL);}
     	|	LCBR 			{Manager_block_open();} 
		stmts RCBR 		{$$ = Manager_block_close($3);}
	;

funcname:	ID 			{$$ = Manager_funcname_id($1);}	
	|				{$$ = Manager_funcname_empty();}
	;

funcprefix:	FUNC funcname 		{$$ = Manager_funcprefix($2); }
	  ;

funcargs:	LPAR idlist RPAR 	{Manager_funcargs();}
	;

funcblockstart:				{Manager_funcblockstart();}
	      ;

funcblockend:				{Manager_funcblockend();}
	    ;

funcbody:	 funcblockstart block funcblockend {$$ = Manager_funcbody($2);}
	;

funcdef:	N funcprefix funcargs funcbody {$$ = Manager_funcdef($1, $2,$4);}
       ;

const:		INT 			{$$ = Manager_const_int($1);}  		
     	| 	REAL 			{$$ = Manager_const_real($1);} 		
	| 	STRING 			{$$ = Manager_const_string($1);}	
	| 	NIL 			{$$ = Manager_const_nil();}		
	| 	TRUE 			{$$ = Manager_const_boolean(true);}
	| 	FALSE 			{$$ = Manager_const_boolean(false);}	     	
	;

idlist:     	ID 			{Manager_idlist($1);} 
      	|	idlist COMA ID 		{Manager_idlist($3);}
	|	/*empty*/
	;

ifprefix:	IF LPAR expr RPAR 	{ $$ = Manager_ifprefix($3);}
	;

if:		ifprefix stmt 		{ $$ = Manager_if($1,$2);}
      	|	ifprefix stmt ELSE N M stmt {$$ =  Manager_ifelse($1,$2,$4,$5,$6);}
	;

loopstart:				{Manager_loopstart();} 
	 ;

loopend:				{Manager_loopend();}
       ;

loopstmt: 	loopstart stmt loopend 	{ $$ = Manager_loopstmt($2);}
	;

whilestart:	WHILE			{ $$ = Manager_whilestart();}
	  ;

whilecond:	LPAR expr RPAR		{ $$ = Manager_whilecond($2);}
	;

while:		whilestart whilecond loopstmt { Manager_while($1,$2,$3);}
	 ;

N:					{$$ = Manager_n();}
 ;

M:					{$$ = Manager_m();}
 ;

forprefix:	FOR LPAR elist SCOL M expr SCOL { $$ = Manager_forprefix($5,$6);}
	 ;

for:		forprefix N elist RPAR N loopstmt N { Manager_for($1,$2,$5,$6,$7);}
	;

returnstmt:	RET SCOL		{$$ = Manager_return();}
	|	RET expr SCOL		{$$ = Manager_return_expr($2);}
	;
	
%%
/*
*	Epilogos
*/
int main(){
	FATAL_ERROR(SymbolTable_create(), "Cant allocate memory for symbol table");
	push_loop(0);
	yyparse();
/*	print_quads();*/
	instructions_make();
	textfile();
	binaryfile();
	return 0;
}

