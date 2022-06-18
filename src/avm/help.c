/*	File: help.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation 
 */

#include "avm.h"

/****** COMPARISON	******/
comparison_func_t comparisonFuncs[] = {
	jlt_impl,
	jgt_impl,
	jle_impl,
	jge_impl
};

uint8_t jge_impl(double x, double y) {return x >= y;}
uint8_t jgt_impl(double x, double y) {return x > y;}
uint8_t	jle_impl(double x, double y) {return x <= y;}
uint8_t jlt_impl(double x, double y) {return x < y;}

/******	ARITHMETIC	******/
arithmetic_func_t arithmeticFuncs[] = {
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl	
};

double add_impl(double x, double y)	{return x + y;}
double sub_impl(double x, double y)	{return x-y;}
double mul_impl(double x, double y)	{return x*y;}
double div_impl(double x, double y){
	FATAL_ERROR(y,"Cant div with 0");
	return x/y;
}


double mod_impl(double x, double y){
	FATAL_ERROR(y,"Cant mod with 0");
	return (uint32_t)x % (uint32_t)y;
}



/****** AVM_TOSTRING	******/
char* avm_tostring(AVM_memcell* m){
	FATAL_ERROR(!( (m->type>=0) && (m->type==undef_m) ),"Invalid memcell");
	  return (*tostringFuncs[m->type])(m);

}

tostring_func_t	tostringFuncs[] = {
	number_tostring,
	string_tostring,
	bool_tostring,
	table_tostring,
	userfunc_tostring,
	libfunc_tostring,
	nil_tostring,
	undef_tostring
};

char* number_tostring(AVM_memcell* m){
	double tmp;
	int i=0;	
	char* ret;
	tmp = m->data.numVal;
	while(tmp){
		++i;
		tmp = tmp/10;
	}
	ret = malloc(i+1);
	sprintf(ret,"%f", m->data.numVal);
	return ret;
}

char* string_tostring(AVM_memcell* m){
	return strdup(m->data.strVal);
}

char* bool_tostring(AVM_memcell* m){
	char* ret;
	switch(m->data.boolVal){
		case 0:	{
			ret = strdup("false");
			break;}
		case 1: {
			ret = strdup("true");
			break;}
		default:{
			FATAL_ERROR(0,"Bool memcell must have bool value");}
	}
	return ret;
}


char*  table_tostring(AVM_memcell* m){
    	char ret[5000];
    	char* p = ret;
    	struct AVM_table* table = m->data.tableVal;
    	uint32_t i;
	p = p + sprintf(p, "[");	
    	AVM_table_bucket* tmp;
    	for (i = 0; i < AVM_TABLE_HASHSIZE; i++) {
        	tmp = table->numIndexed[i];
        	while(tmp){
            		p += sprintf(p,"{%s : %s}, ",avm_tostring(&tmp->key),avm_tostring(&tmp->value));
            		tmp = tmp->next;
        	}
    	}
    	for (i = 0; i < AVM_TABLE_HASHSIZE; i++) {
        	tmp = table->strIndexed[i];
        	while(tmp){
            		p += sprintf(p,"{%s : %s}, ",avm_tostring(&tmp->key),avm_tostring(&tmp->value));
            		tmp = tmp->next;
        	}
    	}
    	for (i = 0; i < AVM_TABLE_HASHSIZE; i++) {
        	tmp = table->ufuncIndexed[i];
        	while(tmp){
            		p += sprintf(p,"{%s : %s}, ",avm_tostring(&tmp->key),avm_tostring(&tmp->value));
            		tmp = tmp->next;
        	}	
    	}
    	for (i = 0; i < AVM_TABLE_HASHSIZE; i++) {
        	tmp = table->lfuncIndexed[i];
        	while(tmp){
            		p += sprintf(p,"{%s : %s}, ",avm_tostring(&tmp->key),avm_tostring(&tmp->value));
            		tmp = tmp->next;
        	}
    	}
    	for (i = 0; i < 2; i++) {
        	tmp = table->boolIndexed[i];
        	while(tmp){
            		p += sprintf(p,"{%s : %s}, ",avm_tostring(&tmp->key),avm_tostring(&tmp->value));
            		tmp = tmp->next;
        	}
    	}
    	p = p + sprintf(p, "]");
    	return strdup(ret);
}

char* userfunc_tostring(AVM_memcell* m){
	uint32_t i=0;
	uint32_t addr, tmp;
	char ret[100];
	char *pret = ret;
	char pre[] = "User Function in address ";
	char* caddr;
	FATAL_ERROR(m->data.funcVal,"Func memcell must have func value");
	while(i<userFuncs_curr){
		if(userFuncs[i].address == m->data.funcVal){ addr = userFuncs[i].address; }
		++i;
	}
	if(addr){
		i=1;	
		tmp = addr;
		while(tmp){
			++i;
			tmp = tmp/10;
		}
		caddr = malloc(i);
		sprintf(caddr,"%u", addr);
		pret = pret + sprintf(ret,"%s%s", pre, caddr);
		return strdup(ret);
	}
	return NULL;
}

char* libfunc_tostring(AVM_memcell* m){
	FATAL_ERROR(m->data.libfuncVal,"LibFunc memcell must have libfunc value");
	char ret[100];
	char *pret = ret;
	char pre[] = "Library Function with name ";
	pret = pret + sprintf(ret,"%s%s", pre, strdup(m->data.libfuncVal));
	return strdup(ret);
}

char* nil_tostring(AVM_memcell* m){
	return strdup("nil");
}

char* undef_tostring(AVM_memcell* m){
	return strdup("undef");
}


/******	AVM_TOBOOL	******/
uint8_t avm_tobool(AVM_memcell* m){
	FATAL_ERROR((m->type>=0 && m->type<undef_m), "Memcell out of bounds");
	return (*toboolFuncs[m->type])(m);
}

tobool_func_t toboolFuncs[] = {
	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};

uint8_t number_tobool(AVM_memcell* m)	{return m->data.numVal != 0;}
uint8_t	string_tobool(AVM_memcell* m)	{return m->data.strVal[0] != 0;}
uint8_t	bool_tobool(AVM_memcell* m)	{return m->data.boolVal;}
uint8_t	table_tobool(AVM_memcell* m)	{return 1;}
uint8_t	userfunc_tobool(AVM_memcell* m)	{return 1;}
uint8_t	libfunc_tobool(AVM_memcell* m)	{return 1;}
uint8_t	nil_tobool(AVM_memcell* m)	{return 0;}
uint8_t	undef_tobool(AVM_memcell* m){
	FATAL_ERROR(0,"Cant bool undef");
	return 0;
}


char* typeStrings[] = {
	"number",
	"string",
	"bool",
	"table",
	"userfunc",
	"libfunc",
	"nil",
	"undef"
};
