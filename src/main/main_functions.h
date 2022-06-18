/*	File: main_functions.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface 
 */

#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include "../avm/avm.h"

Instruction 	*instructions;

extern void avm_initialize(void);
extern void execute_cycle(void);

int 		binaryfile_read(char* file_name);
void	 	binaryarrays_read(FILE* file);
void		binaryinstr_read(FILE* file);


void		avm_textfile(void);
void 		avm_textarrays(FILE* file);
void 		avm_textop(uint32_t i, FILE* file);
void 		avm_textinstr(FILE* file);
#endif
