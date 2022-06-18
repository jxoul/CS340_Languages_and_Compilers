/*	File: avm.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation 
 */

#include "avm.h"

uint32_t totalActuals = 0;

/******	AVM_HELP	******/
AVM_memcell* avm_translate_operand(AVM_arg* arg, AVM_memcell* reg){

	switch(arg->type){
		case global_a:	{return &stack[AVM_STACKSIZE-1-(arg->val)];}
		case local_a:	{return &stack[topsp-(arg->val)];}
		case formal_a:	{return &stack[topsp+AVM_STACKENV_SIZE+1+(arg->val)];}
		case retval_a:	{return &retval;}
		case number_a:	{
					reg->type = number_m;
					reg->data.numVal = numConsts[arg->val];
					return reg;
				}
		case string_a:	{
					reg->type = string_m;
					reg->data.strVal = strdup(stringConsts[arg->val]);
					return reg;
				}
		case bool_a:	{
					reg->type = bool_m;
					reg->data.boolVal = arg->val;
					return reg;
				}
		case nil_a:	{
					reg->type = nil_m;
					return reg;
				}
		case userfunc_a:{
					reg->type = userfunc_m;
					reg->data.funcVal = userFuncs[arg->val].address;
					return reg;
				}
		case libfunc_a:{
					reg->type = libfunc_m;
					reg->data.libfuncVal = namedLibfuncs[arg->val];
					return reg;
				}
		default:	{	FATAL_ERROR(0,"Translate default");}
	}
}


void avm_assign(AVM_memcell* lv, AVM_memcell* rv){
	if(lv == rv){ return;}
	if( lv->type == table_m && rv->type == table_m && lv->data.tableVal == rv->data.tableVal){return;}
	if(rv->type == undef_m){ fprintf(stderr, "Assigning from 'undef' content!\n");}
	avm_memcell_clear(lv);
	memcpy(lv,rv,sizeof(AVM_memcell));

	if(lv->type == string_m){ 
		lv->data.strVal = strdup(rv->data.strVal);}
	else if(lv->type == table_m){ 
		avm_tablerc_inc(lv->data.tableVal);}
	else if(lv->type == libfunc_m){
		lv->data.libfuncVal = strdup(rv->data.libfuncVal);}
}


Userfunc* avm_getfuncinfo(uint32_t address){
	uint32_t i=0, found=0;
	while( i<userFuncs_curr ){
		if(userFuncs[i].address == address){
			found=1;
			break;
		}
		++i;
	}
	if(found){ return &userFuncs[i];}
	else{ return NULL;}
}

uint32_t avm_totalactuals(){
	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

AVM_memcell* avm_getactual(uint32_t i){
	FATAL_ERROR((i<avm_totalactuals()), "Actual out of scope");
	return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}

void finish_program(){
	executionFinished = 1;
}

/******	AVM_STACK_ENV	******/
void avm_initialize(void){
	avm_initstack();
}

static void avm_initstack(){
	uint32_t i;
	for( i=0; i<AVM_STACKSIZE; ++i){
		AVM_WIPEOUT(stack[i]);
		stack[i].type = undef_m;
	}
}

void avm_callsaveenvironment(){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc+1);
	avm_push_envvalue(top+totalActuals+2);
	avm_push_envvalue(topsp);
}

void avm_dec_top(){
	if(!top){
		fprintf(stderr,"Stack Overflow\n");
		finish_program();
	}else{
		--top;
	}
}

void avm_push_envvalue(uint32_t val){
	stack[top].type = number_m;
	stack[top].data.numVal = val;
	avm_dec_top();
}

uint32_t avm_get_envvalue(uint32_t i){
	FATAL_ERROR((stack[i].type == number_m), "Enviroment memcell must have type number_m");
	uint32_t val = (uint32_t) stack[i].data.numVal;
	FATAL_ERROR((stack[i].data.numVal == ((double)val)),"Rule");
	return val;
}

/****** AVM_TABLE	******/
AVM_table* avm_table_new(){
	AVM_table *t =(AVM_table*) malloc(sizeof(AVM_table));
	AVM_WIPEOUT(*t);

	t->refCounter = t->totalStr = t->totalNum = t->totalUfunc = t->totalLfunc = t->totalBool = 0;
	avm_tablebucket_init(t->numIndexed);
	avm_tablebucket_init(t->strIndexed);
	avm_tablebucket_init(t->ufuncIndexed);
	avm_tablebucket_init(t->lfuncIndexed);
	t->boolIndexed[0] = (AVM_table_bucket*) 0;
	t->boolIndexed[1] = (AVM_table_bucket*) 0;
	return t;
}

void avm_table_destroy(AVM_table* t){
	avm_tablebucket_destroy(t->strIndexed);
	avm_tablebucket_destroy(t->numIndexed);
	avm_tablebucket_destroy(t->ufuncIndexed);
	avm_tablebucket_destroy(t->lfuncIndexed);
	avm_tablebucket_destroy(t->boolIndexed);
	free(t);
}

AVM_memcell* avm_tablelem_get(AVM_table* t, AVM_memcell* key){
    	AVM_table_bucket* tmp;
    	AVM_memcell* found;
    	uint32_t hash, i=0;
    	switch(key->type) {
        	case number_m:{
        		hash = hash_double(key->data.numVal);
            		tmp = t->numIndexed[hash];
            		while (tmp) {
                		if(key->data.numVal == tmp->key.data.numVal) {
                    			return &tmp->value;
                		}
                		tmp = tmp->next;
            		}
            		break;}
        	case string_m:{
            		hash = hash_name(key->data.strVal);
            		tmp = t->strIndexed[hash];
            		while(tmp){
                		if(!strcmp(key->data.strVal,tmp->key.data.strVal)){
                    			return &tmp->value;
                		}
                		tmp = tmp->next;
            		}
            		break;}
		case bool_m:{   
			if(key->data.boolVal){hash = 1;}
			else{ hash = 0;}
			tmp = t->boolIndexed[hash];
			while(tmp){
				if(key->data.boolVal == tmp->key.data.boolVal){
					return &tmp->value;
				}
				tmp = tmp->next;
			}
			break;}
		case userfunc_m:{
			hash = hash_uint(key->data.funcVal);
			tmp = t->ufuncIndexed[hash];
			while(tmp){
				if(key->data.funcVal == tmp->key.data.funcVal){
					return &tmp->value;
				}
				tmp = tmp->next;
			}
			break;}
		case libfunc_m:{
			hash = hash_name(key->data.libfuncVal);
			tmp = t->lfuncIndexed[hash];
			while(tmp){
				if(!strcmp(key->data.strVal,tmp->key.data.libfuncVal)){
					return &tmp->value;
				}
				tmp = tmp->next;
			}
			break;}
        	default: {break;}
    	}	
	found = malloc(sizeof(AVM_memcell));
    			found->type = nil_m;
    			return found;

}


void avm_tablelem_set(AVM_table* t, AVM_memcell* key, AVM_memcell* value){
	AVM_memcell *lookup = avm_tablelem_get(t,key);
	uint32_t hash;
	AVM_table_bucket *tmp, *curr;
		
	if(lookup->type != nil_m){
		avm_assign(lookup,value);
	}else{
		switch(key->type){
			case number_m:{
					
					hash = hash_double(key->data.numVal);
					tmp = malloc(sizeof(AVM_table_bucket));
					memset(tmp,0,sizeof(AVM_table_bucket));
					avm_assign(&tmp->key,key);
					avm_assign(&tmp->value, value);
					curr = t->numIndexed[hash];	
					tmp->next = NULL;
					if(curr){
						while(curr->next){
							curr = curr->next;
						}
						curr->next = tmp;
					}else{
						t->numIndexed[hash] = tmp;	
					}
					++t->totalNum;
					break;
				      }
			case string_m:{
					hash = hash_name(key->data.strVal);
					tmp = malloc(sizeof(AVM_table_bucket));
					memset(tmp,0,sizeof(AVM_table_bucket));
					avm_assign(&tmp->key,key);
					avm_assign(&tmp->value, value);
					/*tmp->next = t->strIndexed[hash];
					t->strIndexed[hash] = tmp;*/
					curr = t->strIndexed[hash];	
					tmp->next = NULL;
					if(curr){
						while(curr->next){
							curr = curr->next;
						}
						curr->next = tmp;
					}else{
						t->strIndexed[hash] = tmp;	
					}

					
					++t->totalStr;
					break;
				      }
			case bool_m:{
					if(key->data.boolVal){hash = 1;}
					else{ hash = 0;}
					tmp = malloc(sizeof(AVM_table_bucket));
					memset(tmp,0,sizeof(AVM_table_bucket));
					avm_assign(&tmp->key,key);
					avm_assign(&tmp->value, value);
					/*tmp->next = t->boolIndexed[hash];
					t->boolIndexed[hash] = tmp;*/
					curr = t->boolIndexed[hash];	
					tmp->next = NULL;
					if(curr){
						while(curr->next){
							curr = curr->next;
						}
						curr->next = tmp;
					}else{
						t->boolIndexed[hash] = tmp;	
					}

					++t->totalBool;
					break;
				      }
			case userfunc_m:{
					hash = hash_uint(key->data.funcVal);
					tmp = malloc(sizeof(AVM_table_bucket));
					memset(tmp,0,sizeof(AVM_table_bucket));
					avm_assign(&tmp->key,key);
					avm_assign(&tmp->value, value);
				/*	tmp->next = t->ufuncIndexed[hash];
					t->ufuncIndexed[hash] = tmp;
				*/
					curr = t->ufuncIndexed[hash];	
					tmp->next = NULL;
					if(curr){
						while(curr->next){
							curr = curr->next;
						}
						curr->next = tmp;
					}else{
						t->ufuncIndexed[hash] = tmp;	
					}

					++t->totalLfunc;
					break;
				      }
			case libfunc_m:{
					hash = hash_name(key->data.libfuncVal);
					tmp = malloc(sizeof(AVM_table_bucket));
					memset(tmp,0,sizeof(AVM_table_bucket));
					avm_assign(&tmp->key,key);
					avm_assign(&tmp->value, value);
					/*tmp->next = t->lfuncIndexed[hash];
					t->lfuncIndexed[hash] = tmp;*/
					curr = t->lfuncIndexed[hash];	
					tmp->next = NULL;
					if(curr){
						while(curr->next){
							curr = curr->next;
						}
						curr->next = tmp;
					}else{
						t->lfuncIndexed[hash] = tmp;	
					}

					++t->totalUfunc;
					break;
				      }
			default: return;
		}
		
	}
}


void avm_tablerc_inc(AVM_table* t){
	++t->refCounter;
}

void avm_tablerc_dic(AVM_table* t){
	FATAL_ERROR((t->refCounter > 0), "Table ref counter negative");
	if(!--t->refCounter){
		avm_table_destroy(t);
	}
}

void avm_tablebucket_init(AVM_table_bucket** p){
	uint32_t i;
	for(i=0; i<AVM_TABLE_HASHSIZE; ++i){
		p[i] = (AVM_table_bucket*) 0;
	}	
}

void avm_tablebucket_destroy(AVM_table_bucket** p){
	uint32_t i;
	AVM_table_bucket *tmp, *del;
	for(i=0; i<AVM_TABLE_HASHSIZE; ++i){
		tmp = p[i];
		while(tmp){
			del = tmp;
			tmp = tmp->next;
			avm_memcell_clear(&del->key);
			avm_memcell_clear(&del->value);
			free(del);
		}
		p[i] = (AVM_table_bucket*) 0;
	}
}

void avm_memcell_clear(AVM_memcell* m){
	memclear_func_t f;
	if(m->type != undef_m){
		f = memclearFuncs[m->type];
		if(f){ (*f)(m);}	
		m->type = undef_m;
	}
}

void memclear_string(AVM_memcell* m){
	FATAL_ERROR((m->data.strVal),"String memcell must have strVal");
	free(m->data.strVal);
}

void memclear_table(AVM_memcell* m){
	FATAL_ERROR((m->data.tableVal),"Table memcell must have tableVal");
	avm_tablerc_dic(m->data.tableVal);
}


memclear_func_t	memclearFuncs[] = {
	0,
	memclear_string,
	0,
	memclear_table,
	0,
	0,
	0,
	0
};

static uint32_t hash_name(char* name){
	uint32_t ui, uiHash=0U;
	for(ui=0U; name[ui] != '\0'; ++ui){
		uiHash = uiHash * AVM_HASH_MULTIPLIER + name[ui];
	}
	return uiHash % AVM_TABLE_HASHSIZE;
}

static uint32_t hash_double(double num){
	return (uint32_t)num * AVM_HASH_MULTIPLIER % AVM_TABLE_HASHSIZE;
}

static uint32_t hash_uint(uint32_t num){
	return num * AVM_HASH_MULTIPLIER % AVM_TABLE_HASHSIZE;
}

