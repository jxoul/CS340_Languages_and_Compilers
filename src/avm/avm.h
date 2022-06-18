/*	File: avm.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface 
 */

#ifndef AVM_H
#define AVM_H

#include "arch.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define AVM_STACKSIZE		4096
#define AVM_WIPEOUT(m)		memset(&(m), 0 , sizeof(m))
#define AVM_STACKENV_SIZE 	4
#define AVM_ENDING_PC		codeSize
#define AVM_MAX_INSTRUCTIONS 	(uint32_t) nop_v
#define AVM_HASH_MULTIPLIER	65599
#define FATAL_ERROR(x,y) 	{if(!x){fprintf(stderr,"FATAL ERROR: %s: at line:%d, file:%s \n", y, __LINE__, __FILE__); exit(EXIT_FAILURE);}}
#define PI			3.1415

#define AVM_NUMACTUALS_OFFSET 	+4
#define AVM_SAVEDPC_OFFSET 	+3
#define AVM_SAVEDTOP_OFFSET 	+2
#define AVM_SAVEDTOPSP_OFFSET 	+1

#define execute_add	execute_arithmetic
#define execute_sub	execute_arithmetic
#define execute_mul	execute_arithmetic
#define execute_div	execute_arithmetic
#define execute_mod	execute_arithmetic

#define execute_jlt	execute_comparison
#define execute_jgt	execute_comparison
#define execute_jge	execute_comparison
#define execute_jle	execute_comparison

AVM_memcell 	stack[AVM_STACKSIZE];
AVM_memcell 	ax, bx, cx, retval;

Instruction 	*instructions;
double 		*numConsts;
char 		**stringConsts;
char 		**namedLibfuncs;
Userfunc	*userFuncs;

uint32_t	numConsts_curr;
uint32_t	stringConsts_curr;
uint32_t	namedLibfuncs_curr;
uint32_t	userFuncs_curr;

uint8_t		executionFinished;
uint32_t 	pc, currLine, codeSize;
uint32_t 	top, topsp;
uint32_t	totalActuals;

Instruction*	code;

/******	AVM_HELP	******/
AVM_memcell* 	avm_translate_operand(AVM_arg* arg, AVM_memcell* reg);
void		avm_assign(AVM_memcell* lv, AVM_memcell* rv);
Userfunc*	avm_getfuncinfo(uint32_t address);
uint32_t	avm_totalactuals(void);
AVM_memcell*	avm_getactual(uint32_t i);
void		finish_program(void);

/******	AVM_STACK_ENV	******/
void		avm_initialize(void);
static void 	avm_initstack(void);
void		avm_callsaveenvironment(void);
void		avm_dec_top(void);
void		avm_push_envvalue(uint32_t val);
uint32_t	avm_get_envvalue(uint32_t i);


/****** AVM_TABLE	******/
AVM_table*	avm_table_new(void);
void		avm_table_destroy(AVM_table* t);
AVM_memcell*	avm_tablelem_get(AVM_table* t, AVM_memcell* key);
void		avm_tablelem_set(AVM_table* t, AVM_memcell* key, AVM_memcell* value); 
void		avm_tablerc_inc(AVM_table* t);
void		avm_tablerc_dic(AVM_table* t);
void		avm_tablebucket_init(AVM_table_bucket** p);
void		avm_tablebucket_destroy(AVM_table_bucket** p);
void		avm_memcell_clear(AVM_memcell* m);
void		memclear_string(AVM_memcell* m);
void		memclear_table(AVM_memcell* m);
static uint32_t hash_name(char* name);
static uint32_t hash_double(double num);
static uint32_t hash_uint(uint32_t num);

typedef void 	(*memclear_func_t)(AVM_memcell*);
extern memclear_func_t	memclearFuncs[];

/****** COMPARISON	******/
typedef uint8_t	(*comparison_func_t)(double x, double y);
extern comparison_func_t comparisonFuncs[];

uint8_t		jge_impl(double x, double y);
uint8_t		jgt_impl(double x, double y);
uint8_t		jle_impl(double x, double y);
uint8_t		jlt_impl(double x, double y);

/******	ARITHMETIC	******/
typedef double	(*arithmetic_func_t)(double x, double y);
extern arithmetic_func_t arithmeticFuncs[];

double		add_impl(double x, double y);
double		sub_impl(double x, double y);
double		mul_impl(double x, double y);
double		div_impl(double x, double y);
double		mod_impl(double x, double y);

/****** AVM_TOSTRING	******/
char*		avm_tostring(AVM_memcell* m);

typedef char*	(*tostring_func_t)(AVM_memcell* m);
extern tostring_func_t	tostringFuncs[];

char*		number_tostring(AVM_memcell* m);
char*		string_tostring(AVM_memcell* m);
char*		bool_tostring(AVM_memcell* m);
char*		table_tostring(AVM_memcell* m);
char*		userfunc_tostring(AVM_memcell* m);
char*		libfunc_tostring(AVM_memcell* m);
char*		nil_tostring(AVM_memcell* m);
char*		undef_tostring(AVM_memcell* m);


extern char*	typeStrings[];

/******	AVM_TOBOOL	******/
uint8_t		avm_tobool(AVM_memcell* m);

typedef uint8_t	(*tobool_func_t)(AVM_memcell*);
extern tobool_func_t	toboolFuncs[];

uint8_t		number_tobool(AVM_memcell* m);
uint8_t		string_tobool(AVM_memcell* m);
uint8_t		bool_tobool(AVM_memcell* m);
uint8_t		table_tobool(AVM_memcell* m);
uint8_t		userfunc_tobool(AVM_memcell* m);
uint8_t		libfunc_tobool(AVM_memcell* m);
uint8_t		nil_tobool(AVM_memcell* m);
uint8_t		undef_tobool(AVM_memcell* m);

/****** EXECUTE 	******/
void		execute_cycle(void);

typedef void	(*execute_func_t)(Instruction* instr);
extern execute_func_t	executeFuncs[];

void 		execute_assign(Instruction* instr);
void 		execute_arithmetic(Instruction* instr);
void		execute_jump(Instruction* instr);
void 		execute_jeq(Instruction* instr);
void 		execute_jne(Instruction* instr);
void 		execute_jle(Instruction* instr);
void 		execute_jge(Instruction* instr);
void 		execute_jlt(Instruction* instr);
void 		execute_jgt(Instruction* instr);
void 		execute_call(Instruction* instr);
void 		execute_pusharg(Instruction* instr);
void 		execute_funcenter(Instruction* instr);
void 		execute_funcexit(Instruction* instr);
void 		execute_newtable(Instruction* instr);
void 		execute_tablegetelem(Instruction* instr);
void 		execute_tablesetelem(Instruction* instr);
void 		execute_nop(Instruction* instr);

/******	LIBRARY FUNCS	******/
typedef void	(*library_func_t)(void);
extern library_func_t	avm_libraryfunc_get(char* id);

void		avm_libfunc_call(char* id);
void		avm_libfunc_register(char* id, library_func_t addr);
void 		libfunc_print(void);
void		libfunc_typeof(void);
void		libfunc_totalarguments(void);
void 		libfunc_input(void);
void 		libfunc_objectmemberkeys(void);
void 		libfunc_objecttotalmembers(void);
void 		libfunc_objectcopy(void);
void 		libfunc_argument(void);
void 		libfunc_strtonum(void);
void 		libfunc_sqrt(void);
void 		libfunc_cos(void);
void 		libfunc_sin(void);

/******	CONSTS ARRAYS	******/
/*** NUMBER ***/

uint32_t 	numConsts_next(void);

double		numConsts_get(uint32_t index);

/*** STRING ***/

uint32_t 	stringConsts_next(void);

char*		stringConsts_get(uint32_t index);

/*** LIBFUNCS ***/

uint32_t 	namedLibfuncs_next(void);

char*		namedLibfuncs_get(uint32_t index);

/*** USERFUNCS ***/

uint32_t 	userFuncs_next(void);

uint32_t	userFuncs_get(uint32_t index);

#endif
