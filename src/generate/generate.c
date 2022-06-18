/*      File: generate.c
 *      Authors: Ioannis Xoulidis csd3410
 *               Stavros Karaxristianidis csd3434
 *      Design: Interface Implementation
 */

#include "generate.h"

void generate(AVM_opcode opcode, Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;
	t.opcode = opcode;
	make_operand(quad->arg1, &t.arg1);
	make_operand(quad->arg2, &t.arg2);
	make_operand(quad->result, &t.result);
	t.srcLine = quad->line;
	quad->taddress = instructions_next();
	instructions_emit(t);
}

void generate_relational(AVM_opcode opcode, Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;
	t.opcode = opcode;
	make_operand(quad->arg1, &t.arg1);
	make_operand(quad->arg2, &t.arg2);
	t.result.type = label_a;
	t.srcLine = quad->line;
	
	if(quad->label < processedquad_next()){
		t.result.val = quads[quad->label].taddress;
	}else{
		ij_add(instructions_next(), quad->label);
	}

	quad->taddress = instructions_next();
	instructions_emit(t);
}

void generate_ASSIGN(Quad* quad){ generate(assign_v, quad);}
void generate_ADD(Quad* quad){ 	generate(add_v, quad);}
void generate_SUB(Quad* quad){ generate(sub_v, quad);}
void generate_MUL(Quad* quad){ generate(mul_v, quad);}
void generate_DIV(Quad* quad){ generate(div_v, quad);}
void generate_MOD(Quad* quad){ generate(mod_v, quad);}
void generate_NEWTABLE(Quad* quad){ generate(newtable_v, quad);}
void generate_TABLEGETELEM(Quad* quad){ generate(tablegetelem_v, quad);}
void generate_TABLESETELEM(Quad* quad){ generate(tablesetelem_v, quad);}

void generate_NOP(){
	Instruction t;
	t.opcode = nop_v;
	instructions_emit(t);
}

void generate_JUMP(Quad* quad){ generate_relational(jump_v, quad);}
void generate_IF_EQ(Quad* quad){ generate_relational(jeq_v, quad);}
void generate_IF_NOTEQ(Quad* quad){ generate_relational(jne_v, quad);}
void generate_IF_GREATER(Quad* quad){ generate_relational(jgt_v, quad);}
void generate_IF_GREATEREQ(Quad* quad){ generate_relational(jge_v, quad);}
void generate_IF_LESS(Quad* quad){ generate_relational(jlt_v, quad);}
void generate_IF_LESSEQ(Quad* quad){ generate_relational(jle_v, quad);}

void generate_PARAM(Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;

	quad->taddress = instructions_next();
	t.opcode = pusharg_v;
	t.srcLine = quad->line;
	make_operand(quad->arg1, &t.result);
	instructions_emit(t);
}

void generate_CALL(Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;
	quad->taddress = instructions_next();
	t.opcode = call_v;
	make_operand(quad->arg1, &t.result);
	instructions_emit(t);
}

void generate_GETRETVAL(Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;
	quad->taddress = instructions_next();
	t.opcode = assign_v;
	make_operand(quad->result, &t.result);
	make_retvaloperand(&t.arg1);
	t.srcLine = quad->line;
	instructions_emit(t);
}

void generate_FUNCSTART(Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;
	quad->taddress = instructions_next();
	t.opcode = funcenter_v;
	make_operand(quad->result, &t.result);
	t.srcLine = quad->line;
	instructions_emit(t);
}

void generate_RETURN(Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;
	quad->taddress = instructions_next();
	t.opcode = assign_v;
	make_operand(quad->result, &t.arg1);
	make_retvaloperand(&t.result);
	t.srcLine = quad->line;
	instructions_emit(t);
}

void generate_FUNCEND(Quad* quad){
	Instruction t;
	t.result.type = 7;
	t.arg1.type = 7;
	t.arg2.type = 7;

	quad->taddress = instructions_next();
	t.opcode = funcexit_v;
	make_operand(quad->result, &t.result);
	t.srcLine = quad->line;
	instructions_emit(t);
}

void generate_UMINUS(Quad* quad){
	quad->arg2 = newexpr_constnum(-1);
	generate(mul_v, quad);
}


