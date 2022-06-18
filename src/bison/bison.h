/*	File: bison.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: ADT
 */
#ifndef BISON_H
#define BISON_H


#include <stdint.h>

typedef enum{
	false, true
} Boolean;

typedef enum{
	var_s, programfunc_s, libraryfunc_s
} Symbol_T;

typedef enum{
	program_var, function_local, formal_arg
} Scopespace_T;

typedef enum{
	var_e, tableitem_e, programfunc_e, libraryfunc_e, arithexpr_e, boolexpr_e, 
	assignexpr_e, newtable_e, constnum_e, constbool_e, 
	conststring_e, nil_e
} Expression_T;

typedef enum{
	assign=0 , add, sub, mul, Div, mod, uminus, and, or, not, if_eq, 
	if_noteq, if_lesseq, if_greatereq, if_less, if_greater,jump, call, 
	param, ret, getretval, funcstart, funcend, tablecreate, 
	tablegetelem, tablesetelem
} Iopcode;

typedef struct SymbolTableEntry{
	Boolean 	isActive;
	char 		*name;
	Scopespace_T 	space;
	uint32_t 	offset;
	uint32_t 	scope;
	uint32_t 	line;
	uint32_t	iaddress;
	uint32_t 	totalLocals;
	Symbol_T 	type;
	struct SymbolTableEntry *nextc;
	struct SymbolTableEntry *nexts;
} SymbolTableEntry;

typedef struct Expression{
	Expression_T	type;
	SymbolTableEntry *sym;
	struct Expression *index;
	double		numConst;
	char		*strConst;
	Boolean 	boolConst;
	struct Uint	*tlist;
	struct Uint	*flist;
	struct Expression *next;
	struct Expression *prev;
} Expression;


typedef struct Quad{
	Iopcode op;
	Expression *result;
	Expression *arg1;
	Expression *arg2;
	uint32_t label;
	uint32_t line;
	uint32_t taddress;
} Quad;

typedef struct Call {
	Expression* elist;
	uint8_t method;
	char* name;
} Call;

typedef struct Indexed{
	Expression* key;
	Expression* value;
	struct Indexed *next;
} Indexed;

struct SymbolTable{
	SymbolTableEntry** hashTable;
	SymbolTableEntry** scopeTable;
	uint32_t totalSymbols;
} *symtable;

typedef struct stacknode{
	uint32_t offset;
	struct stacknode* next;
} snode;

typedef struct Uint{
	uint32_t num;
	struct Uint *next;
} Uint;

typedef struct Statement{
	Uint* blist;
	Uint* clist;
	Uint* rlist;
} Statement;

typedef struct Forprefix{
	uint32_t test;
	uint32_t enter;
} Forprefix;

snode *stackoffset;
Uint *stackloop;
SymbolTableEntry *lookupEntry;
#endif
