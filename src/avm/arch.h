/*      File: arch.h
 *      Authors: Ioannis Xoulidis csd3410
 *               Stavros Karaxristianidis csd3434
 *      Design: ADT
 */

#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>
#define AVM_TABLE_HASHSIZE 211

typedef enum {
	assign_v, add_v , sub_v, mul_v ,  div_v , mod_v , 
	jump_v, jeq_v, jne_v, jlt_v, jgt_v, jle_v, jge_v, 
	call_v, pusharg_v, funcenter_v , funcexit_v,
	newtable_v , tablegetelem_v , tablesetelem_v , 
	nop_v
} AVM_opcode;

typedef enum {
	label_a=0, global_a, formal_a, local_a, number_a, string_a, 
	bool_a, nil_a, userfunc_a, libfunc_a, retval_a
} AVM_arg_t;

typedef enum {
	number_m=0, string_m, bool_m, table_m, userfunc_m, libfunc_m,
	nil_m, undef_m
} AVM_memcell_t;

typedef struct AVM_arg {
	AVM_arg_t 	type;
	uint32_t 	val;
} AVM_arg;

typedef struct Instruction {
	AVM_opcode  	opcode;
	AVM_arg 	result;
	AVM_arg		arg1;
	AVM_arg		arg2;
	uint32_t	srcLine;
} Instruction;

typedef struct Userfunc {
	uint32_t 	address;
	uint32_t	localSize;
	char*		id;
} Userfunc;

typedef struct Incomplete_jump{
	uint32_t instrNo;
	uint32_t iaddress;
	struct Incomplete_jump* next;
} Incomplete_jump;

typedef struct AVM_memcell {
	AVM_memcell_t	type;
	union{
		double		numVal;
		char		*strVal;
		uint8_t		boolVal;
		struct AVM_table *tableVal;
		uint32_t	funcVal;
		char		*libfuncVal;	
	} data;
} AVM_memcell;

typedef struct AVM_table_bucket {
	AVM_memcell	key;
	AVM_memcell	value;
	struct AVM_table_bucket *next;
} AVM_table_bucket;

typedef struct AVM_table {
	uint32_t refCounter;
	AVM_table_bucket *strIndexed[AVM_TABLE_HASHSIZE];
	uint32_t totalStr;
	AVM_table_bucket *numIndexed[AVM_TABLE_HASHSIZE];
	uint32_t totalNum;
	AVM_table_bucket *ufuncIndexed[AVM_TABLE_HASHSIZE];
	uint32_t totalUfunc;
	AVM_table_bucket *lfuncIndexed[AVM_TABLE_HASHSIZE];
	uint32_t totalLfunc;
	AVM_table_bucket *boolIndexed[2];
	uint32_t totalBool;
} AVM_table;

#endif
