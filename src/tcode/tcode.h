/*	File: tcode.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface 
 */

#ifndef TCODE_H
#define TCODE_H

#include "../funcs/funcs.h"
#include "../avm/arch.h"
#include "../generate/generate.h"

#define CURR_INSTR_SIZE 	(instructions_total*sizeof(Instruction))
#define NEW_INSTR_SIZE 		(EXPAND_SIZE*sizeof(Instruction)+CURR_INSTR_SIZE)

#define CURR_NUM_SIZE 		(numConsts_total*sizeof(double))
#define NEW_NUM_SIZE 		(EXPAND_SIZE*sizeof(double)+CURR_NUM_SIZE)       

#define CURR_STR_SIZE 		(stringConsts_total*sizeof(char*))
#define NEW_STR_SIZE 		(EXPAND_SIZE*sizeof(char*)+CURR_STR_SIZE)       

#define CURR_UFUNC_SIZE 	(userFuncs_total*sizeof(Userfunc))
#define NEW_UFUNC_SIZE 		(EXPAND_SIZE*sizeof(Userfunc)+CURR_UFUNC_SIZE)       

#define CURR_LFUNC_SIZE 	(namedLibfuncs_total*sizeof(char*))
#define NEW_LFUNC_SIZE 		(24*sizeof(char*)+CURR_LFUNC_SIZE)       

Instruction 	*instructions;
extern double 		*numConsts;
extern char 		**stringConsts;
extern char 		**namedLibfuncs;
extern Userfunc	*userFuncs;
Incomplete_jump *ij_head;

uint32_t	instructions_curr;
uint32_t	instructions_total;
uint32_t	quads_processed;
extern uint32_t	numConsts_curr;
uint32_t	numConsts_total;
extern uint32_t	stringConsts_curr;
uint32_t	stringConsts_total;
extern uint32_t	namedLibfuncs_curr;
uint32_t	namedLibfuncs_total;
extern uint32_t	userFuncs_curr;
uint32_t	userFuncs_total;
uint32_t	ij_total;

void 		make_operand(Expression* expr, AVM_arg* arg);

/****** PRINT	******/
/*** TEXT FILE ***/
void		textfile(void);
void		textinstr(FILE* file);
void		textop(uint32_t t, FILE* file);
void 		textarrays(FILE* file);

/*** BINARY FILE ***/
void		binaryfile(void);
void		binaryarrays(FILE* file);
void 		binaryinstr(FILE* file);



/****** INCOMPLETE JUMPS ******/
void 		ij_add(uint32_t instrNo, uint32_t iaddress);
void		ij_patch(void);

/******	HELPER FUNCTIONS ******/
void 		make_numberoperand(AVM_arg* arg, double val);
void 		make_booloperand(AVM_arg* arg, Boolean val);
void 		make_retvaloperand(AVM_arg* arg);


/******	CONSTS ARRAYS	******/
/*** NUMBER ***/
uint32_t	numConsts_add(double n);
uint32_t 	numConsts_next(void);
void		numConsts_inc(void);
void 		numConsts_expand(void);
double		numConsts_get(uint32_t index);

/*** STRING ***/
uint32_t	stringConsts_add(char* s);
uint32_t 	stringConsts_next(void);
void		stringConsts_inc(void);
void 		stringConsts_expand(void);
char*		stringConsts_get(uint32_t index);

/*** LIBFUNCS ***/
uint32_t	namedLibfuncs_add(char* s);
uint32_t 	namedLibfuncs_next(void);
void		namedLibfuncs_inc(void);
void 		namedLibfuncs_expand(void);
char*		namedLibfuncs_get(uint32_t index);

/*** USERFUNCS ***/
uint32_t	userFuncs_add(SymbolTableEntry* sym);
uint32_t 	userFuncs_next(void);
void		userFuncs_inc(void);
void 		userFuncs_expand(void);
uint32_t	userFuncs_get(uint32_t index);

/******	INSTRUCTIONS	******/
void		instructions_make(void);
uint32_t 	instructions_next(void);
void		instructions_inc(void);
void 		instructions_expand(void);
void		instructions_emit(Instruction i);

uint32_t	processedquad_next(void);
void		processedquad_inc(void);

#endif
