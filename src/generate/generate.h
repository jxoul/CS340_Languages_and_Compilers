/*      File: generate.h
 *      Authors: Ioannis Xoulidis csd3410
 *               Stavros Karaxristianidis csd3434
 *      Design: Interface
 */

#ifndef		GENERATE_H
#define		GENERATE_H

#include "../tcode/tcode.h"
#include "../avm/arch.h"

void generate(AVM_opcode opcode, Quad* quad);
void generate_relational(AVM_opcode opcode, Quad* quad);

void generate_ADD(Quad* quad);
void generate_SUB(Quad* quad);
void generate_MUL(Quad* quad);
void generate_DIV(Quad* quad);
void generate_MOD(Quad* quad);
void generate_NEWTABLE(Quad* quad);
void generate_TABLEGETELEM(Quad* quad);
void generate_TABLESETELEM(Quad* quad);
void generate_ASSIGN(Quad* quad);
void generate_NOP();
void generate_JUMP(Quad* quad);
void generate_IF_EQ(Quad* quad);
void generate_IF_NOTEQ(Quad* quad);
void generate_IF_GREATER(Quad* quad);
void generate_IF_GREATEREQ(Quad* quad);
void generate_IF_LESS(Quad* quad);
void generate_IF_LESSEQ(Quad* quad);
void generate_PARAM(Quad* quad);
void generate_CALL(Quad* quad);
void generate_GETRETVAL(Quad* quad);
void generate_FUNCSTART(Quad* quad);
void generate_RETURN(Quad* quad);
void generate_FUNCEND(Quad* quad);
void generate_UMINUS(Quad* quad);

#endif
