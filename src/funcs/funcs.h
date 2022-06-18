/*	File: funcs.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface 
 */

#ifndef FUNCS_H
#define FUNCS_H

#include "../bison/bison.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FATAL_ERROR(x,y) 	{if(!x){fprintf(stderr,"FATAL ERROR: %s: at line:%d, file:%s \n", y, __LINE__, __FILE__); exit(EXIT_FAILURE);}}
#define CURR_SIZE (total*sizeof(Quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(Quad)+CURR_SIZE)

#define MAX_BUCKETS 	257
#define MAX_SCOPES 	257	       
#define EXPAND_SIZE 	1024


int yylex(void);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

extern uint32_t access;
extern uint32_t programVarOffset;
extern uint32_t functionLocalOffset;
extern uint32_t formalArgOffset;
extern uint32_t scopeSpaceCounter;
extern uint32_t curr_scope;
extern uint32_t fnn;
extern uint32_t total;
extern uint32_t currQuad;
extern uint32_t tempCounter;
extern uint32_t loopcounter;
extern Quad *quads;

/*******************	BACKPATCHING		**************/
void backpatch(Uint* list, uint32_t label);

/*******************	STACK			**************/
void push_offset(uint32_t offset);
uint32_t pop_offset(void);

void push_loop(uint32_t loop);
uint32_t pop_loop(void);

/*******************	EXPRESSIONS		**************/
Expression* newexpr(Expression_T type);
Expression* newexpr_conststring(char* str);
Expression* newexpr_constnum(double num);
Expression* newexpr_constbool(Boolean b);
Expression* lvalue_expr(SymbolTableEntry* sym);
Expression* member_item( Expression* lv, char* name);
uint32_t check_arith(Expression* expr);
Uint* merge_tlist(Expression* expr1, Expression* expr2);
Uint* merge_flist(Expression* expr1, Expression* expr2);
void expr_eval(Expression* expr);

/*******************	MAKE			**************/
Expression* make_call(Expression* lvalue, Expression* reversed_elist);
void make_tflist(Expression* expr);

/*******************	STATEMENT		**************/
Statement* new_stmt(uint32_t opt);
Uint* new_uint(uint32_t num);

/*******************	QUAD			**************/
void expand(void);
void emit(Iopcode op, Expression* result, Expression* arg1, Expression* arg2, uint32_t label, uint32_t line); 
Expression* emit_iftableitem(Expression* e);
uint32_t nextquadlabel(void);
void incquad(void);
void patchlabel(uint32_t quadNo, uint32_t label);
void patch_blist(Statement* stmt, uint32_t label);
void patch_clist(Statement* stmt, uint32_t label);
void patch_rlist(Statement* stmt, uint32_t label);

/*******************	PRINT			**************/
void print_quads(void);
char* get_expr_name(Expression* e);
char* float_to_string(float f);
int n_tu(int number, int count);

/*******************	TEMP		*********************/
char* newtempname(void);
SymbolTableEntry* newtemp(void);
void resetemp(void);
Boolean istempname(char* name);
Boolean istempexpr(Expression* expr);

/******************	SCOPE OFFSET SPACE	**************/

/* Current Scope */	
uint32_t currscope_get(void);	
void currscope_inc(void);
void currscope_dic(void);

/* Scope Space */
void scopespace_inc(void);
Scopespace_T scopespace_get(void);
void scopespace_dic(void);

/* Scope Offset */
uint32_t scopeoffset_get(void);
void scopeoffset_inc(void);
void scopeoffset_restore(uint32_t n);

void formalargoffset_reset(void);
void functionlocaloffset_reset(void);

/* Loop */
Boolean inLoop(void);
Boolean inFunction(void);
uint32_t getloop(void);
void setloop(uint32_t loop);
void resetloop(void);
void enterloop(void);
void exitloop(void);
void ploop();

/*******************		REST	**********************/
void yyerror (char* yaccProvidedMessage);
char* nonameFunc(void);
uint32_t powerof(uint32_t x); 
char* inttostr(int value, char* result, int base); 
Statement* merge(Statement* stmt1, Statement* stmt2);

/*******************	SYMBOL TABLE	**********************/
uint32_t hash(const char * name);
Boolean SymbolTable_create(void);
SymbolTableEntry* SymbolTableEntry_create(char* name, uint32_t scope, uint32_t line, Symbol_T type);
void SymbolTable_insert(SymbolTableEntry *entry);
void SymbolTable_init(void);
void SymbolTable_print(void);
void SymbolTable_hide(uint32_t scope);
SymbolTableEntry* SymbolTable_Clookup(char* name);
SymbolTableEntry* SymbolTable_Slookup(char* name, uint32_t scope);
SymbolTableEntry* SymbolTable_lookup(char* name, int flag);
Boolean lib_func_found(char* name);
Boolean symisFunc(SymbolTableEntry* entry);
Boolean exprisFunc(Expression* expr);

#endif
