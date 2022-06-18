/*	File: libraryfunctions.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation
 */

#include "avm.h"
#include <math.h>
#include <stdlib.h>

library_func_t	avm_libraryfunc_get(char* id){
	if(!strcmp(id, "print")){		return libfunc_print;}
	if(!strcmp(id, "input")){		return libfunc_input;}
	if(!strcmp(id, "objectmemberkeys")){	return libfunc_objectmemberkeys;}
	if(!strcmp(id, "objecttotalmembers")){	return libfunc_objecttotalmembers;}
	if(!strcmp(id, "objectcopy")){		return libfunc_objectcopy;}
	if(!strcmp(id, "totalarguments")){	return libfunc_totalarguments;}
	if(!strcmp(id, "argument")){		return libfunc_argument;}
	if(!strcmp(id, "typeof")){		return libfunc_typeof;}
	if(!strcmp(id, "strtonum")){		return libfunc_strtonum;}
	if(!strcmp(id, "sqrt")){		return libfunc_sqrt;}
	if(!strcmp(id, "cos")){			return libfunc_cos;}
	if(!strcmp(id, "sin")){			return libfunc_sin;}
	FATAL_ERROR(0, "Wrong libfunc name");
}

void avm_libfunc_call(char* id){
	library_func_t f = avm_libraryfunc_get(id);
	if(!f){
		fprintf(stderr,"Unsupported lib func '%s' called", id);
		finish_program();
	}else{
		topsp = top;
		totalActuals = 0;
		(*f)();
		if(!executionFinished){
			execute_funcexit((Instruction*) 0);
		}
	}	
}

void libfunc_print(){
	char* s;
	uint32_t i;
	uint32_t n = avm_totalactuals();
	for(i=0; i<n; ++i){
		s = avm_tostring(avm_getactual(i));
		puts(s);
		free(s);	
	}	
}
 
 
void libfunc_typeof(){
	uint32_t n = avm_totalactuals();	
	if( n!=1){
		fprintf(stderr,"One argument (not %u) expected in 'typeof'", n);
		finish_program();
	}else{
		avm_memcell_clear(&retval);
		retval.type = string_m;
		retval.data.strVal = strdup(typeStrings[avm_getactual(0)->type]);
	}
}

void libfunc_totalarguments(){
	uint32_t p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcell_clear(&retval);

	if(p_topsp == AVM_STACKSIZE-1){
		retval.type = nil_m;
	}else{
		retval.type = number_m;
		retval.data.numVal = avm_get_envvalue(p_topsp+ AVM_NUMACTUALS_OFFSET);
	}
}

void libfunc_input(){}

void libfunc_objectmemberkeys(){
	AVM_table *newtable = avm_table_new();
	AVM_table *oldtable;
	AVM_memcell *arg, *newkey;
	AVM_table_bucket *currbucket;
	int i=0, index=0;

	FATAL_ERROR((avm_totalactuals()==1), "Cant use more than 1 arguments at objectmemberskeys function");
	arg = avm_getactual(0);
	FATAL_ERROR((arg->type == table_m),"Invalid parameter at objectmemberskeys function (must be table)" );
	oldtable = arg->data.tableVal;


	for(i=0; i<AVM_TABLE_HASHSIZE; ++i){
		currbucket = oldtable->strIndexed[i];
		while(currbucket){
			newkey = malloc(sizeof(AVM_memcell));
			newkey->type = number_m;
			newkey->data.numVal = index;
			avm_tablelem_set(newtable, newkey,&currbucket->key);
			currbucket = currbucket->next;
			++index;
		}
	}
	
	for(i=0; i<AVM_TABLE_HASHSIZE; ++i){
		currbucket = oldtable->numIndexed[i];
		while(currbucket){
			newkey = malloc(sizeof(AVM_memcell));
			newkey->type = number_m;
			newkey->data.numVal = index;
			avm_tablelem_set(newtable, newkey,&currbucket->key);
			currbucket = currbucket->next;
			++index;
		}
	}


	for(i=0; i<AVM_TABLE_HASHSIZE; ++i){
		currbucket = oldtable->ufuncIndexed[i];
		while(currbucket){
			newkey = malloc(sizeof(AVM_memcell));
			newkey->type = number_m;
			newkey->data.numVal = index;
			avm_tablelem_set(newtable, newkey,&currbucket->key);
			currbucket = currbucket->next;
			++index;
		}
	}

	for(i=0; i<AVM_TABLE_HASHSIZE; ++i){
		currbucket = oldtable->lfuncIndexed[i];
		while(currbucket){
			newkey = malloc(sizeof(AVM_memcell));
			newkey->type = number_m;
			newkey->data.numVal = index;
			avm_tablelem_set(newtable, newkey,&currbucket->key);
			currbucket = currbucket->next;
			++index;
		}
	}

	for(i=0; i<2; ++i){
		currbucket = oldtable->boolIndexed[i];
		while(currbucket){
			newkey = malloc(sizeof(AVM_memcell));
			newkey->type = number_m;
			newkey->data.numVal = index;
			avm_tablelem_set(newtable, newkey,&currbucket->key);
			currbucket = currbucket->next;
			++index;
		}
	}


	retval.type = table_m;
	retval.data.tableVal = newtable;
}

void libfunc_objecttotalmembers(){
	AVM_memcell *arg;
	AVM_table *table;
	FATAL_ERROR((avm_totalactuals()==1), "Cant use more than 1 arguments at objecttotalmembers function");
	arg = avm_getactual(0);
	avm_memcell_clear(&retval);
	FATAL_ERROR((arg->type == table_m),"Invalid parameter at objecttotalmembers function (must be table)" );
	table = arg->data.tableVal;
	retval.type = number_m;
	retval.data.numVal = table->totalStr + table->totalNum + table->totalUfunc + table->totalLfunc + table->totalBool;
}

void libfunc_objectcopy(){}

void libfunc_argument(){
	AVM_memcell* actual;
	uint32_t  p_topsp;
	int i;
	if(avm_totalactuals() == 1){
		actual = avm_getactual(0);
		FATAL_ERROR((actual->type == number_m), "Parameter to argument function must be number");
		i = actual->data.numVal;
		FATAL_ERROR((i+1<=avm_totalactuals()),"Invalid parameter to argument (does not exist)");
		FATAL_ERROR((i>=0), "Invalid parameter to argument (must me positive)");
		
	fprintf(stderr,"%u\n", i);
		p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET) + 4 + 1 + i;
		if(p_topsp == AVM_STACKSIZE-1){
			retval.type = nil_m;
		}else{

			/*retval.type = number_m;
			retval.data.numVal = avm_get_envvalue(p_topsp+ AVM_STACKENV_SIZE + i);
			*/avm_assign(&retval, &stack[p_topsp]);
		}

	}else{
		fprintf(stderr,"Cant use more than 1 arguments at argument function");
		finish_program();
	}
}


void libfunc_strtonum(){
    FATAL_ERROR((avm_totalactuals()==1), "Cant use more than 1 arguments at objecttotalmembers function");
    AVM_memcell* arg = avm_getactual(0);
    FATAL_ERROR((arg->type == string_m), "Parameter to strtonum function must be number");
    retval.type = number_m;
    retval.data.numVal = atoi(arg->data.strVal);
}

void libfunc_sqrt(){
	AVM_memcell *x;
	FATAL_ERROR((avm_totalactuals()==1), "Cant use more than 1 arguments at sqrt function");
	avm_memcell_clear(&retval);

	x = avm_getactual(0);
	FATAL_ERROR((x->type == number_m), "Parameter to sqrt function must be number");
	if(x->data.numVal >= 0){
		retval.type = number_m;
		retval.data.numVal = sqrt(x->data.numVal);
	}else{
		fprintf(stderr,"Cant use negative arguments at sqrt function\n");
		retval.type = nil_m;	
	}
}

void libfunc_cos(){
	AVM_memcell *x;
	FATAL_ERROR((avm_totalactuals()==1), "Cant use more than 1 arguments at cos function");
	avm_memcell_clear(&retval);

	x = avm_getactual(0);
	FATAL_ERROR((x->type == number_m), "Parameter to cos function must be number");
	retval.type = number_m;
	retval.data.numVal = cos(x->data.numVal * ( PI/180));
}

void libfunc_sin(){
	AVM_memcell *x;
	FATAL_ERROR((avm_totalactuals()==1), "Cant use more than 1 arguments at sin function");
	avm_memcell_clear(&retval);

	x = avm_getactual(0);
	FATAL_ERROR((x->type == number_m), "Parameter to sin function must be number");
	retval.type = number_m;
	retval.data.numVal = sin(x->data.numVal * ( PI/180));
}
