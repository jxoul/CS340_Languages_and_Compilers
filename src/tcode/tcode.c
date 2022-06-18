/*	File: tcode.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation
 */

#include "tcode.h"

Instruction 	*instructions = (Instruction*) 0;
double 		*numConsts = (double*) 0;
char 		**stringConsts = (char**) 0;
char 		**namedLibfuncs = (char**) 0;
Userfunc	*userFuncs = (Userfunc*) 0;
Incomplete_jump *ij_head = (Incomplete_jump*) 0;

uint32_t	instructions_curr = 0;
uint32_t	instructions_total = 0;
uint32_t	quads_processed = 0;
uint32_t	numConsts_curr = 0;
uint32_t	numConsts_total = 0;
uint32_t	stringConsts_curr = 0;
uint32_t	stringConsts_total = 0;
uint32_t	namedLibfuncs_curr = 0;
uint32_t	namedLibfuncs_total = 0;
uint32_t	userFuncs_curr = 0;
uint32_t	userFuncs_total = 0;
uint32_t	ij_total = 0;

void make_operand(Expression* expr, AVM_arg* arg){
	if(expr){
		switch(expr->type){
			case var_e:
			case tableitem_e:
			case arithexpr_e:
			case boolexpr_e:
			case assignexpr_e:
			case newtable_e: {
				FATAL_ERROR((expr->sym), "Expression must have symbol");
				arg->val = expr->sym->offset;
				switch(expr->sym->space){
					case 0: 	{arg->type = global_a; break;}
					case 1:		{arg->type = local_a; break;}
					case 2:		{arg->type = formal_a; break;}
					default:		FATAL_ERROR(0,"DEFAULT");
				}
				break;
			}
			case constbool_e: {
				arg->type = bool_a;
				arg->val = expr->boolConst;
				break;
			}
			case conststring_e: {
				arg->type = string_a;
				arg->val = stringConsts_add(expr->strConst);
				break;
			}
			case constnum_e: {
				arg->type = number_a;
				arg->val = numConsts_add(expr->numConst);
				break;
			}
			case nil_e: {
				arg->type = nil_a;
				break;
			}
			case programfunc_e: {
				arg->type = userfunc_a;
				arg->val = userFuncs_add(expr->sym);
				break;
			}
			case libraryfunc_e: {
				arg->type = libfunc_a;
				arg->val = namedLibfuncs_add(expr->sym->name);
				break;
			}
			default:
				FATAL_ERROR(0,"Expression illegal type");
		}
	}
}

/****** INCOMPLETE JUMPS ******/
void ij_add(uint32_t instrNo, uint32_t iaddress){
	Incomplete_jump* result = malloc(sizeof(Incomplete_jump));
	result->instrNo = instrNo;
	result->iaddress = iaddress;
	result->next = ij_head;
	ij_head = result;
}

void ij_patch(){
	Incomplete_jump* tmp = ij_head;
	while(tmp){
		if(tmp->iaddress == nextquadlabel()){
			instructions[tmp->instrNo].result.val = instructions_next();		
		}else{
			instructions[tmp->instrNo].result.val = quads[tmp->iaddress].taddress;			
		}
		tmp = tmp->next;
	}
}

/******	HELPER FUNCTIONS ******/
void make_numberoperand(AVM_arg* arg, double val){
	arg->type = number_a;
	arg->val = numConsts_add(val);
}

void make_booloperand(AVM_arg* arg, Boolean val){
	arg->type = bool_a;
	arg->val = (uint32_t) val;
}

void make_retvaloperand(AVM_arg* arg){
	arg->type = retval_a;
}

/******	CONSTS ARRAYS	******/
/*** NUMBER ***/
uint32_t numConsts_add(double n){
        uint32_t ret, i=0;
        while(i<numConsts_next()){
                if( n == numConsts_get(i)){ return i;}
                i++;
        }
        if(numConsts_total == numConsts_next()){
                numConsts_expand();
        }
        ret = numConsts_next();
        numConsts[ret] = n;
        numConsts_inc();
        return ret;
}


uint32_t numConsts_next(){
	return numConsts_curr;
}

void numConsts_inc(){
	++numConsts_curr;
}

void numConsts_expand(){
	double *result = (double*) malloc(NEW_NUM_SIZE);
	FATAL_ERROR(result, "Cant allocate enough memory for number expand");
	if(numConsts){
		memcpy(result,numConsts,CURR_NUM_SIZE);
		free(numConsts);
	}
	numConsts = result;
	numConsts_total += EXPAND_SIZE;
}

double numConsts_get(uint32_t index){
	return numConsts[index];
}

/*** STRING ***/

uint32_t stringConsts_add(char* string){
        uint32_t ret,i=0;
	int len,j, newl;
	char *kati, *p;
	
       	while(i < stringConsts_next()){
                if( !strcmp(string, stringConsts_get(i))) { return i;}
                i++;
        }

	if(stringConsts_total == stringConsts_next()){
                stringConsts_expand();
        }

        ret = stringConsts_next();
	
	if( string[0] == '\"'){ 
		len = strlen(string);
		kati = malloc(len-1);
		for(j=1; j<=len-2; ++j){
			kati[j-1] = string[j];
		}
		kati[len-2] = '\0';
        	stringConsts[ret] = strdup(kati);
	}else{
        	stringConsts[ret] = strdup(string);
	}

       

        stringConsts_inc();
        return ret;
}

uint32_t stringConsts_next(){
	return stringConsts_curr;
}

void stringConsts_inc(){
	++stringConsts_curr;
}

void stringConsts_expand(){
	char **result = (char**) malloc(NEW_STR_SIZE);
	FATAL_ERROR(result, "Cant allocate enough memory for string expand");
	if(stringConsts){
		memcpy(result,stringConsts,CURR_STR_SIZE);
		free(stringConsts);
	}
	stringConsts = result;
	stringConsts_total += EXPAND_SIZE;
}

char* stringConsts_get(uint32_t index){
	return stringConsts[index];
}

/*** LIBFUNCS ***/
uint32_t namedLibfuncs_add(char* string){
	uint32_t ret, i=0;
	while(i<namedLibfuncs_next()){
		if(!strcmp(string,namedLibfuncs[i])){
			return i;
		}
		i++;
	}

	if(namedLibfuncs_total == namedLibfuncs_next()){
		namedLibfuncs_expand();
	}
	ret = namedLibfuncs_next();
	namedLibfuncs[ret] = strdup(string);
	namedLibfuncs_inc();
	return ret;
}

uint32_t namedLibfuncs_next(){
	return namedLibfuncs_curr; 
}

void namedLibfuncs_inc(){
	++namedLibfuncs_curr;
}

void namedLibfuncs_expand(){
	char **result = (char**) malloc(NEW_LFUNC_SIZE);
	FATAL_ERROR(result, "Cant allocate enough memory for library funcs expand");
	if(namedLibfuncs){
		memcpy(result,namedLibfuncs,CURR_LFUNC_SIZE);
		free(namedLibfuncs);
	}
	namedLibfuncs = result;
	namedLibfuncs_total += 24;
}

char* namedLibfuncs_get(uint32_t index){
	return namedLibfuncs[index];
}


/*** USERFUNCS ***/
uint32_t userFuncs_add(SymbolTableEntry* sym){
	uint32_t ret,i=0;
	Userfunc* tmp =userFuncs;
	while(i<userFuncs_next()){
		if((!strcmp(sym->name,userFuncs[i].id)) && (sym->iaddress == userFuncs[i].address)){
		return i;
		}
		i++;
	}
	if(userFuncs_total == userFuncs_next()){
		userFuncs_expand();
	}
	Userfunc *result = malloc(sizeof(Userfunc));
	memset(result,0,sizeof(Userfunc));
	result->address = sym->iaddress;
	result->localSize = sym->totalLocals;
	result->id = strdup(sym->name);
	ret = userFuncs_next();
	userFuncs[ret] = *result;
	userFuncs_inc();
	return ret;
}

uint32_t userFuncs_next(){
	return userFuncs_curr;
}

void userFuncs_inc(){
	++userFuncs_curr;
}

void userFuncs_expand(){
	Userfunc *result = (Userfunc*) malloc(NEW_UFUNC_SIZE);
	FATAL_ERROR(result, "Cant allocate enough memory for userfuncs expand");
	if(userFuncs){
		memcpy(result,userFuncs,CURR_UFUNC_SIZE);
		free(userFuncs);
	}
	userFuncs = result;
	userFuncs_total += EXPAND_SIZE;
}


uint32_t userFuncs_get(uint32_t index){
	return userFuncs[index].address;
}

/******	INSTRUCTIONS	******/
void instructions_make(){
	while(processedquad_next() < nextquadlabel()){
		switch(quads[processedquad_next()].op){
			case 0: { generate_ASSIGN(&quads[processedquad_next()]); break;}
			case 1: { generate_ADD(&quads[processedquad_next()]); break;}
			case 2: { generate_SUB(&quads[processedquad_next()]); break;}
			case 3: { generate_MUL(&quads[processedquad_next()]); break;}
			case 4: { generate_DIV(&quads[processedquad_next()]); break;}
			case 5: { generate_MOD(&quads[processedquad_next()]); break;}
			case 6: { generate_UMINUS(&quads[processedquad_next()]); break;}
			case 10: { generate_IF_EQ(&quads[processedquad_next()]); break;}
			case 11: { generate_IF_NOTEQ(&quads[processedquad_next()]); break;}
			case 12: { generate_IF_LESSEQ(&quads[processedquad_next()]); break;}
			case 13: { generate_IF_GREATEREQ(&quads[processedquad_next()]); break;}
			case 14: { generate_IF_LESS(&quads[processedquad_next()]); break;}
			case 15: { generate_IF_GREATER(&quads[processedquad_next()]); break;}
			case 16: { generate_JUMP(&quads[processedquad_next()]); break;}
			case 17: { generate_CALL(&quads[processedquad_next()]); break;}
			case 18: { generate_PARAM(&quads[processedquad_next()]); break;}
			case 19: { generate_RETURN(&quads[processedquad_next()]); break;}
			case 20: { generate_GETRETVAL(&quads[processedquad_next()]); break;}
			case 21: { generate_FUNCSTART(&quads[processedquad_next()]); break;}
			case 22: { generate_FUNCEND(&quads[processedquad_next()]); break;}
			case 23: { generate_NEWTABLE(&quads[processedquad_next()]); break;}
			case 24: { generate_TABLEGETELEM(&quads[processedquad_next()]); break;}
			case 25: { generate_TABLESETELEM(&quads[processedquad_next()]); break;}
			default: { fprintf(stderr,"Kati paei lathos");}
		}
		processedquad_inc();
	}
	ij_patch();
}


uint32_t instructions_next(){
	return instructions_curr;
}

void instructions_inc(){
	++instructions_curr;
}

void instructions_expand(){
	FATAL_ERROR((instructions_total==instructions_next()), "Total Instructions");
	Instruction *i = (Instruction*) malloc(NEW_INSTR_SIZE);
	FATAL_ERROR(i, "Cant allocate enough memory for instruction expand");
	if(instructions){
		memcpy(i,instructions,CURR_INSTR_SIZE);
		free(instructions);
	}
	instructions = i;
	instructions_total += EXPAND_SIZE;
}


void instructions_emit(Instruction i){
	if(instructions_total == instructions_next()){
		instructions_expand();
	}
	instructions[instructions_next()] = i;
	instructions_inc();
}



uint32_t processedquad_next(){
	return quads_processed;
}

void processedquad_inc(){
	++quads_processed;
}
