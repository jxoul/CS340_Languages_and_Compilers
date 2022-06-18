/*	File: execute.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation 
 */

#include "avm.h"

uint8_t		executionFinished=0;
uint32_t 	pc=0;
uint32_t	currLine=0;
uint32_t	codeSize=0;

void execute_cycle(){
	uint32_t oldPC;
	Instruction* instr;
	if(executionFinished){ return;}
	else if(pc == AVM_ENDING_PC){
		finish_program();
		return;
	}else{
		FATAL_ERROR((pc<AVM_ENDING_PC), "PC out of bounds");
		instr = instructions + pc;
		FATAL_ERROR((instr->opcode>=0  && instr->opcode<=AVM_MAX_INSTRUCTIONS), "Instruction Opcode out of bounds");
		if(instr->srcLine){
			currLine = instr->srcLine;
		}
		oldPC = pc;
		(*executeFuncs[instr->opcode])(instr);
		if(pc == oldPC){
			++pc;
		}
	}
}

execute_func_t	executeFuncs[] = {
	execute_assign,
	execute_add,
	execute_sub,
	execute_mul,
	execute_div,
	execute_mod,
	execute_jump,
	execute_jeq,
	execute_jne,
	execute_jlt,
	execute_jgt,
	execute_jle,
	execute_jge,
	execute_call,
	execute_pusharg,
	execute_funcenter,
	execute_funcexit,
	execute_newtable,
	execute_tablegetelem,
	execute_tablesetelem,
	execute_nop
};

void execute_assign(Instruction* instr){
	AVM_memcell *lv = avm_translate_operand(&instr->result, (AVM_memcell*) 0);
	AVM_memcell *rv = avm_translate_operand(&instr->arg1, &ax);

	FATAL_ERROR((lv&&rv),"Left and Right value must exist");
	FATAL_ERROR((&stack[AVM_STACKSIZE - 1] >=lv || lv==&retval), "Left value out of bounds");

	avm_assign(lv,rv);


}

void execute_arithmetic(Instruction* instr){
	arithmetic_func_t op;
	AVM_memcell* lv = avm_translate_operand(&instr->result,(AVM_memcell*) 0);
	AVM_memcell* rv1 = avm_translate_operand(&instr->arg1,&ax);
	AVM_memcell* rv2 = avm_translate_operand(&instr->arg2,&bx);

	FATAL_ERROR((lv && rv1 && rv2), "Values must exist");
	FATAL_ERROR((&stack[AVM_STACKSIZE-1]>=lv || lv==&retval), "Rule");

	if(rv1->type!=number_m || rv2->type!=number_m){
		fprintf(stderr,"Not a number in arithmetic\n");
		finish_program();
	}else{
		op = arithmeticFuncs[instr->opcode - add_v];

		avm_memcell_clear(lv);
		lv->type = number_m;
		lv->data.numVal = (*op)(rv1->data.numVal, rv2->data.numVal);
	}
}

void execute_jump(Instruction* instr){
	FATAL_ERROR((instr->result.type == label_a), "Instruction must have label");
	if(!executionFinished){
		pc = instr->result.val;
	}
}

void execute_jeq(Instruction* instr){
	uint8_t result = 0;
	FATAL_ERROR((instr->result.type == label_a), "Instruction must have label");
	AVM_memcell* rv1 = avm_translate_operand(&instr->arg1,&ax);
	AVM_memcell* rv2 = avm_translate_operand(&instr->arg2,&bx);

	if(rv1->type == undef_m || rv2->type == undef_m){
		fprintf(stderr,"'undef' involved in equality\n");
		finish_program();
	}else if(rv1->type == nil_m || rv2->type == nil_m){
		result = rv1->type == nil_m && rv2->type == nil_m;
	}else if(rv1->type == bool_m || rv2->type == bool_m){
		result = (avm_tobool(rv1) == avm_tobool(rv2));
	}else if(rv1->type != rv2->type){
		fprintf(stderr,"%s == %s is illegal\n", typeStrings[rv1->type], typeStrings[rv2->type]);
		finish_program();
	}else if(rv1->type == number_m && rv2->type == number_m){
		result = rv1->data.numVal == rv2->data.numVal;
	}else if(rv1->type == string_m && rv2->type == string_m){
		if(!strcmp(rv1->data.strVal, rv2->data.strVal)){
			result = 1;
		}else{
			result = 0;
		}
	}else{
		result = avm_tobool(rv1);
	}

	if(!executionFinished && result){
		pc = instr->result.val;
	}
}

void execute_jne(Instruction* instr){
	uint8_t result = 0;
	FATAL_ERROR((instr->result.type == label_a), "Instruction must have label");
	AVM_memcell* rv1 = avm_translate_operand(&instr->arg1,&ax);
	AVM_memcell* rv2 = avm_translate_operand(&instr->arg2,&bx);

	if(rv1->type == undef_m || rv2->type == undef_m){
		fprintf(stderr,"'undef' involved in equality\n");
		finish_program();
	}else if(rv1->type == nil_m || rv2->type == nil_m){
		result = !(rv1->type == nil_m && rv2->type == nil_m);
	}else if(rv1->type == bool_m || rv2->type == bool_m){
		result = !(avm_tobool(rv1) == avm_tobool(rv2));
	}else if(rv1->type != rv2->type){
		fprintf(stderr,"%s == %s is illegal\n", typeStrings[rv1->type], typeStrings[rv2->type]);
		finish_program();
	}else if(rv1->type == number_m && rv2->type == number_m){
		result = !(rv1->data.numVal == rv2->data.numVal);
	}else if(rv1->type == string_m && rv2->type == string_m){
		if(!strcmp(rv1->data.strVal, rv2->data.strVal)){
			result = 0;
		}else{
			result = 1;
		}
	}else{
		result = !(avm_tobool(rv1));
	}

	if(!executionFinished && result){
		pc = instr->result.val;
	}
}

void execute_comparison(Instruction* instr){
	comparison_func_t op;
	uint8_t result=0;
	AVM_memcell* rv1 = avm_translate_operand(&instr->arg1,&ax);
	AVM_memcell* rv2 = avm_translate_operand(&instr->arg2,&bx);
	FATAL_ERROR((rv1 && rv2), "Values must exist");
	if(rv1->type!=number_m || rv2->type!=number_m){
		fprintf(stderr,"Not a number in comparsion\n");
		finish_program();
	}else{
		op = comparisonFuncs[instr->opcode - jlt_v];
		result = (*op)(rv1->data.numVal, rv2->data.numVal);
	}

	if(!executionFinished && result){ pc = instr->result.val;}
}

void execute_call(Instruction* instr){
	char* s;
	AVM_memcell* func = avm_translate_operand(&instr->result, &ax);
	FATAL_ERROR(func, "Func must exist to call");
	avm_callsaveenvironment();

	switch(func->type){
		case userfunc_m: {
					pc = func->data.funcVal;
					FATAL_ERROR((pc<AVM_ENDING_PC), "PC out of bounds");
					FATAL_ERROR((instructions[pc].opcode == funcenter_v), "Call must jump to funcenter");
					break;}
		case string_m:	{
					avm_libfunc_call(func->data.strVal);
					break;}
		case libfunc_m:	{
					avm_libfunc_call(func->data.libfuncVal);
					break;}
		default:	{
					s = avm_tostring(func);
					fprintf(stderr,"Call: cannot bind '%s' to function!", s);
				       	free(s);
					finish_program();}
	}
}

void execute_pusharg(Instruction* instr){
	AVM_memcell* arg = avm_translate_operand(&instr->result,&ax);
	FATAL_ERROR(arg, "Argument must exist");
	avm_assign(&stack[top], arg);
	++totalActuals;
	avm_dec_top();
}

void execute_funcenter(Instruction* instr){
	Userfunc* funcInfo;
	AVM_memcell* func = avm_translate_operand(&instr->result,&ax);
	FATAL_ERROR(func,"Function must exist to enter");
	FATAL_ERROR((pc == func->data.funcVal),"Func address must match PC");
	totalActuals = 0;
	funcInfo = avm_getfuncinfo(pc);
	topsp = top;
	top = top - funcInfo->localSize;
}

void execute_funcexit(Instruction* instr){
	uint32_t oldTop = top;
	top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);

	while(++oldTop <= top){
		avm_memcell_clear(&stack[oldTop]);
	}
}

void execute_newtable(Instruction* instr){	
	AVM_memcell* lv = avm_translate_operand(&instr->result,(AVM_memcell*)0);
	FATAL_ERROR(( lv && (&stack[AVM_STACKSIZE-1]>=lv && lv > &stack[top] || lv==&retval)), "Rule");
	avm_memcell_clear(lv);

	lv->type = table_m;
	lv->data.tableVal = avm_table_new();
	avm_tablerc_inc(lv->data.tableVal);
}


void execute_tablegetelem(Instruction* instr){
	char *ts, *is;
	AVM_memcell* content;
	AVM_memcell* lv = avm_translate_operand(&instr->result,(AVM_memcell*)0);
	AVM_memcell* t = avm_translate_operand(&instr->arg1,(AVM_memcell*)0);
	AVM_memcell* i = avm_translate_operand(&instr->arg2,&ax);

	FATAL_ERROR(( lv && (&stack[AVM_STACKSIZE-1]>=lv && lv > &stack[top] || lv==&retval)), "Rule");
	FATAL_ERROR(( t && (&stack[AVM_STACKSIZE-1]>=t && t > &stack[top] )), "Rule");
	FATAL_ERROR(i,"Rule");

	avm_memcell_clear(lv);
	lv->type = nil_m;
	
	if(t->type != table_m){
		fprintf(stderr,"Illegal use of type %s as table\n", typeStrings[t->type]);
		finish_program();
	} else{
		content = avm_tablelem_get(t->data.tableVal, i);
		if(content){
			avm_assign(lv,content);
		}else {
			ts = avm_tostring(t);
			is = avm_tostring(i);
			fprintf(stderr,"%s[%s] not found", ts, is);
			free(ts);
			free(is);
		}	
	}
}

void execute_tablesetelem(Instruction* instr){
	AVM_memcell* t = avm_translate_operand(&instr->arg1,(AVM_memcell*)0);
	AVM_memcell* i = avm_translate_operand(&instr->arg2,&ax);
	AVM_memcell* c = avm_translate_operand(&instr->result,&bx);

	FATAL_ERROR(( t && (&stack[AVM_STACKSIZE-1]>=t && t > &stack[top] )), "Rule");
	FATAL_ERROR(i,"Rule");

	if(t->type != table_m){
		fprintf(stderr,"Illegal use of type %s as table!", typeStrings[t->type]);
		finish_program();
	}else{
		avm_tablelem_set(t->data.tableVal, i , c);
	}
}

void execute_nop(Instruction* instr){}


