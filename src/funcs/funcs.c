/*	File: funcs.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation
 */

#include "funcs.h"

uint32_t access = 0;
uint32_t programVarOffset = 0;
uint32_t functionLocalOffset = 0;
uint32_t formalArgOffset = 0;
uint32_t scopeSpaceCounter = 1;
uint32_t curr_scope = 0;
uint32_t fnn = 0;
uint32_t total = 0;
uint32_t currQuad = 0;
uint32_t tempCounter = 0;
uint32_t loopcounter = 0;

Quad *quads = (Quad*) 0;

/*******************	BACKPATCHING		**************/
void backpatch(Uint* list, uint32_t label){
	Uint* tmp = list;
	while(tmp){
		quads[tmp->num].label = label;
		tmp = tmp->next;
	}
}

/*******************	STACK			**************/
void push_offset(uint32_t offset){
	snode *newnode = malloc(sizeof(snode));
	FATAL_ERROR(newnode, "Cant allocate enough memory for new snode");
	memset(newnode,0,sizeof(snode));
	newnode->offset = offset;
	newnode->next = stackoffset;
	stackoffset = newnode;
}

uint32_t pop_offset(){
	if(!stackoffset){
		FATAL_ERROR(0,"Offset Stack is empty, cant pop");
	}
	snode *tmp = stackoffset;
	uint32_t result = tmp->offset;
	stackoffset = stackoffset->next;
	free(tmp);
	return result;
}

void push_loop(uint32_t loop){
	Uint *newnode = malloc(sizeof(Uint));
	FATAL_ERROR(newnode, "Cant allocate enough memory for new Uint");
	memset(newnode,0,sizeof(Uint));
	newnode->num = loop;
	newnode->next = stackloop;
	stackloop = newnode;
}

uint32_t pop_loop(){
	if(!stackloop){
		FATAL_ERROR(0, "Loop Stack is empty, cant pop");
	}
	Uint *tmp = stackloop;
	uint32_t result = tmp->num;
	stackloop = stackloop->next;
	free(tmp);
	return result;
}

void ploop(){
	Uint* tmp = stackloop;
	while(tmp->next){
		fprintf(stderr,"[%u]-> ", tmp->num);
		tmp = tmp->next;
	}     
	fprintf(stderr,"[%u]\n", tmp->num);
}

/*******************	EXPRESSIONS		**************/
Expression* newexpr(Expression_T type){
	Expression *expr = malloc(sizeof(Expression));
	FATAL_ERROR(expr, "Cant allocate enough memory for new Expression");
	memset(expr,0,sizeof(Expression));
	expr->type = type;
	return expr;
}

Expression* newexpr_constbool(Boolean b){
	Expression *expr = newexpr(constbool_e);
	expr->boolConst = b;
	return expr;
}

Expression* newexpr_conststring(char* str){
	Expression *expr = newexpr(conststring_e);
	expr->strConst = strdup(str);
	return expr;
}

Expression* newexpr_constnum(double num){
	Expression* expr = newexpr(constnum_e);
	expr->numConst = num;
	return expr;
}


Expression* lvalue_expr (SymbolTableEntry* sym){
	FATAL_ERROR(sym, "Cant create expression from null symbol");
	Expression* expr = malloc(sizeof(Expression));
	FATAL_ERROR(expr, "cant allocate enough memory for new expression");
	memset(expr, 0 ,sizeof(Expression));
	expr->next = NULL;
	expr->sym = sym;
	switch(sym->type){
		case var_s:		expr->type = var_e; break;
		case programfunc_s:	expr->type = programfunc_e; break;
		case libraryfunc_s:	expr->type = libraryfunc_e; break;
		default:		FATAL_ERROR(0, "DEFAULT");
	}
	return expr;
}


Expression* member_item(Expression* lvalue, char* name){
	Expression* item = newexpr(tableitem_e);
	lvalue = emit_iftableitem(lvalue);
	item->sym = lvalue->sym;
	item->index = newexpr_conststring(name);
	return item;
}

uint32_t check_arith(Expression* expr){
	if( ((expr->type)==programfunc_e) || ((expr->type)==libraryfunc_e) || ((expr->type)==boolexpr_e) 
		|| ((expr->type)==newtable_e) || ((expr->type)==constbool_e) || ((expr->type)==conststring_e) 
			|| ((expr->type)==nil_e) ){
			return 0;
	}
	return 1;
}

void expr_eval(Expression* expr){
	backpatch(expr->tlist, nextquadlabel());
	backpatch(expr->flist, nextquadlabel()+2);
	emit(assign,newexpr_constbool(true), NULL,expr, 0 , yylineno);
	emit(jump, NULL,NULL,NULL,nextquadlabel()+2, yylineno);	
	emit(assign,newexpr_constbool(false), NULL,expr, 0 , yylineno);
}

/*******************	MAKE		**************/
Expression* make_call(Expression* lvalue, Expression* list){
	Expression* func = emit_iftableitem(lvalue);
	Expression* result = newexpr(var_e);
	Expression* tmp = list;
	if(tmp){
		while(tmp->next){	
			tmp = tmp->next;
		}	
		while(tmp){
			emit(param,tmp,NULL,NULL,0,yylineno);
			tmp = tmp->prev;
		}
	}

	emit(call, func, NULL,NULL,0,yylineno);
	result->sym = newtemp();
	emit(getretval,NULL,NULL,result, 0, yylineno);
	return result;
}

void make_tflist(Expression* expr){
	FATAL_ERROR(expr,"Cant make tflist for NULL expression");
	expr->tlist = new_uint(nextquadlabel());
	expr->flist = new_uint(nextquadlabel()+1);
	emit(if_eq, expr, newexpr_constbool(true), NULL, 0 , yylineno);
	emit(jump,NULL,NULL,NULL,0,yylineno);
}


/*******************	STATEMENT	**************/
Statement* new_stmt(uint32_t opt){
	Statement* result = malloc(sizeof(Statement));
	FATAL_ERROR(result, "Cant allocate enough memory for new Statement");
	memset(result,0,sizeof(Statement));
	
	switch(opt){
		case 1:
			result->blist = new_uint(nextquadlabel());
			break;
		case 2:
			result->clist = new_uint(nextquadlabel());
			break;
		case 3:
			result->rlist = new_uint(nextquadlabel());
			break;
		default:
			FATAL_ERROR(0,"DEFAULT");
	}
	emit(jump, NULL,NULL,NULL,0,yylineno);
	return result;
}

Uint* new_uint(uint32_t num){
	Uint* result = malloc(sizeof(Uint));
	FATAL_ERROR(result, "Cant allocate enough memory for new Uint");
	memset(result,0,sizeof(Uint));
	result->num = num;
	return result;
}


/*******************	QUAD 			*************/
void expand(){
	FATAL_ERROR((total==nextquadlabel()), "Total quads");
	Quad* p = (Quad*) malloc(NEW_SIZE);
	FATAL_ERROR(p,"Cant allocate enough memory for expand");
	if(quads){
		memcpy(p, quads, CURR_SIZE);
		free(quads);
	}
	quads = p;
	total += EXPAND_SIZE;
}

void emit(Iopcode op, Expression* arg1, Expression* arg2 , Expression* result, uint32_t label, uint32_t line){
	if (nextquadlabel() == total){
		expand();
	}

	quads[currQuad].op = op;
	quads[currQuad].result = result;
	quads[currQuad].arg1 = arg1;
	quads[currQuad].arg2 = arg2;
	quads[currQuad].label = label;
	quads[currQuad].line= line;
	quads[currQuad].taddress = 0;
	incquad();
}

Expression* emit_iftableitem(Expression* expr){
	if(expr->type != tableitem_e){
		return expr;
	}
	Expression* result = newexpr(var_e);
	result->sym = newtemp();
	emit(tablegetelem, expr , expr->index,result , 0, yylineno);
	return result;
}

uint32_t nextquadlabel(){
	return currQuad;
}

void incquad(){
	currQuad++;
}

void patchlabel(uint32_t quadNo, uint32_t label){
	FATAL_ERROR(((quadNo<nextquadlabel()) && (!quads[quadNo].label)), "Cant patch label");
	quads[quadNo].label = label;
}

void patch_blist(Statement* stmt, uint32_t label){

	if(stmt){
		Uint* tmp = stmt->blist;
       		while(tmp){
			quads[tmp->num].label = label;
			tmp = tmp->next;
		}
	}
}

void patch_clist(Statement* stmt, uint32_t label){
	if(stmt){
		Uint* tmp = stmt->clist;
       		while(tmp){
			quads[tmp->num].label = label;
			tmp = tmp->next;
		}
	}
}

void patch_rlist(Statement* stmt, uint32_t label){
	if(stmt){
		Uint* tmp = stmt->rlist;
       		while(tmp){
			quads[tmp->num].label = label;
			tmp = tmp->next;
		}
	}
}

Statement* merge(Statement* stmt1, Statement* stmt2){

	Uint* tmp;

	if(!stmt1){return stmt2;}
	else if(!stmt2){return stmt1;}
	else{
		if(stmt1->blist){
			tmp = stmt1->blist;
			while(tmp->next){
				tmp = tmp->next;
			}
			tmp->next = stmt2->blist;
		}else{
			stmt1->blist = stmt2->blist;
		}
		
		if(stmt1->clist){
			tmp = stmt1->clist;
			while(tmp->next){
				tmp = tmp->next;
			}
			tmp->next = stmt2->clist;
		}else{
			stmt1->clist = stmt2->clist;
		}
			
		if(stmt1->rlist){
			tmp = stmt1->rlist;
			while(tmp->next){
				tmp = tmp->next;
			}
			tmp->next = stmt2->rlist;
		}else{
			stmt1->rlist = stmt2->rlist;
		}

		return stmt1;
	}
}

Uint* merge_tlist(Expression* expr1, Expression* expr2){
	FATAL_ERROR(expr1, "Cant merge tlist from NULL Expression");
	FATAL_ERROR(expr2, "Cant merge tlist from NULL Expression");
	Uint* tmp;

	if(!expr1->tlist){return expr2->tlist;}
	else if(!expr2->tlist){return expr1->tlist;}
	else{
		tmp = expr1->tlist;
		while(tmp->next){
			tmp = tmp->next;
		}
		tmp->next = expr2->tlist;
		return expr1->tlist;
	}
}

Uint* merge_flist(Expression* expr1, Expression* expr2){
	FATAL_ERROR(expr1, "Cant merge flist from NULL Expression");
	FATAL_ERROR(expr2, "Cant merge flist from NULL Expression");
	Uint* tmp;

	if(!expr1->flist){return expr2->flist;}
	else if(!expr2->flist){return expr1->flist;}
	else{
		tmp = expr1->flist;
		while(tmp->next){
			tmp = tmp->next;
		}
		tmp->next = expr2->flist;
		return expr1->flist;
	}
}

/*******************		FOR PRINT *****************/
char* get_expr_name(Expression* expr) {
    	if(expr){
		switch(expr->type){
			case constbool_e:	
						if(expr->boolConst){
							return "TRUE";
						}
						return "FALSE";
			case constnum_e:
                if(expr->sym){
                    return expr->sym->name;
                }
                else
                    return float_to_string(expr->numConst);
			case conststring_e:
						return expr->strConst;
			case nil_e:
						return "nil";
			default:
						return expr->sym->name;
		}						
	}
        return "";
}

void print_quads(void) {
	char* op;
    	fprintf(stdout,"\n\nquad#\topcode\t\tresult\t\targ1\t\targ2\t\tlabel\n");
    	fprintf(stdout,"----------------------------------------------------------------------------------\n");
    	for(int i = 0; i < currQuad; i++) {
        	switch (quads[i].op) {
			case 0: op ="assign"; break;
			case 1: op ="add"; break;
			case 2: op ="sub"; break;
			case 3: op ="mul"; break;
			case 4: op ="div"; break;
			case 5: op ="mod"; break;
			case 6: op ="uminus"; break;
			case 7: op ="and"; break;
			case 8: op ="or"; break;
			case 9: op ="not"; break;
			case 10: op ="if_eq"; break;
			case 11: op ="if_noteq"; break;
			case 12: op ="if_lesseq"; break;
			case 13: op ="if_greatereq"; break;
			case 14: op ="if_less"; break;
			case 15: op ="if_greater"; break;
			case 16: op ="jump"; break;
			case 17: op ="call"; break;
			case 18: op ="param"; break;
			case 19: op ="ret"; break;
			case 20: op ="getretval"; break;
			case 21: op ="funcstart"; break;
			case 22: op ="funcend"; break;
			case 23: op ="tablecreate"; break;
			case 24: op ="tablegetelem"; break;
			case 25: op ="tablesetelem"; break;
			default: FATAL_ERROR(0,"DEFAULT");
		}
		if ((quads[i].op == 11) || (quads[i].op == 12) || (quads[i].op == 13) || (quads[i].op == 15) || (quads[i].op == 20) || (quads[i].op == 21) || (quads[i].op == 23) || (quads[i].op == 24) ||  (quads[i].op == 25)){
			if(quads[i].label == 0){
				fprintf(stdout,"%d:\t%s\t%s\t\t%s\t\t%s\n",i,op,get_expr_name(quads[i].result),get_expr_name(quads[i].arg1),get_expr_name(quads[i].arg2));
			}else{
				fprintf(stdout,"%d:\t%s\t%s\t\t%s\t\t%s\t\t%u\n",i,op,get_expr_name(quads[i].result),get_expr_name(quads[i].arg1),get_expr_name(quads[i].arg2),quads[i].label);
			}
		}else{
			if((quads[i].label == 0)   && (quads[i].op != jump)) {
				fprintf(stdout,"%d:\t%s\t\t%s\t\t%s\t\t%s\n",i,op,get_expr_name(quads[i].result),get_expr_name(quads[i].arg1),get_expr_name(quads[i].arg2));
			}else{
				fprintf(stdout,"%d:\t%s\t\t%s\t\t%s\t\t%s\t\t%u\n",i,op,get_expr_name(quads[i].result),get_expr_name(quads[i].arg1),get_expr_name(quads[i].arg2),quads[i].label);
			}
		}
    	}
}

int n_tu(int number, int count){
    	int result = 1;
    	while((count--) > 0){
        	result *= number;
    	}
    	return result;
}


char* float_to_string(float f){

    float tmp = f;
    int i = 1;
    while(tmp) {
        i++;
        tmp = tmp/10;
    }
    char* r = malloc(i);
    sprintf(r,"%f",f);
    	return r;
}



/*******************	TEMP		**************/
SymbolTableEntry* newtemp(){
	char* name = newtempname();
	SymbolTableEntry* sym = SymbolTable_lookup(name,2);
	if(!sym){
		sym = SymbolTableEntry_create(name, currscope_get(), yylineno, var_s);
		SymbolTable_insert(sym);
	}
	return sym;
}

char* newtempname() {
    	uint32_t num_digits=0, num, remaining, digit=0, index=2, dia;
    	char* str=malloc(6);

    	str[0] = '_';
    	str[1] = 't';
        str[2] = '\0';
        str[3] = '\0';
        str[4] = '\0';
        str[5] = '\0';
    	if(!tempCounter){
        	str[2] = '0';
        	str[3] = '\0';
        	tempCounter++;
        	return str;
    	}
    	num = tempCounter;
    	remaining= tempCounter;
    	while(num>0){
        	num=num/10;
        	num_digits++;
    	}
    	dia = powerof(num_digits);
    	while(num_digits>0){
        	digit = remaining / powerof(num_digits-1);
        	remaining = remaining % powerof(num_digits-1);
        	str[index] = 48 + digit;
        	num_digits--;
        	index++;
    	}
    	str[5] = '\0';
    	tempCounter++;
    	return str;
}

void resetemp(){
	tempCounter = 0;
}

Boolean istempname(char* name){
	return (*name == '_');
}

Boolean istempexpr(Expression* expr){
	return ((expr->sym) && istempname(expr->sym->name));
}

/*******************	SCOPE OFFSET SPACE QUAD	**************/

/* Current Scope */
uint32_t currscope_get(){
	return curr_scope;
}

void currscope_inc(){
	curr_scope++;
}

void currscope_dic(){
	curr_scope--;
}


/* Scope Space */
Scopespace_T scopespace_get(){
	if (scopeSpaceCounter == 1){
		return program_var;
	}else{
		if (scopeSpaceCounter%2 == 0){
			return formal_arg;
		}else{
			return function_local;
		}
	}
}


void scopespace_inc(){
	++scopeSpaceCounter;
}

void scopespace_dic(){
	FATAL_ERROR((scopeSpaceCounter>1), "Error scope space");
	--scopeSpaceCounter;
}


/* Scope Offset */
uint32_t scopeoffset_get(){
	switch(scopespace_get()){
		case program_var:
			return programVarOffset;
		case function_local:
			return functionLocalOffset;
		case formal_arg:
			return formalArgOffset;
		default:
			assert(0);
	}
}

void scopeoffset_inc(){
	switch(scopespace_get()){
		case program_var:
			++programVarOffset; break;
		case function_local:
			++functionLocalOffset; break;
		case formal_arg:
			++formalArgOffset; break;
		default:
			assert(0);
	}
}

void scopeoffset_restore(uint32_t n){
	switch(scopespace_get()){
		case program_var:
			programVarOffset=n; break;
		case function_local:
			functionLocalOffset=n; break;
		case formal_arg:
			formalArgOffset=n; break;
		default:
			assert(0);
	}
}


void formalargoffset_reset(){
	formalArgOffset = 0;
}

void functionlocaloffset_reset(){
	functionLocalOffset = 0;
}

/* Loop */
Boolean inLoop(){
	if (getloop() > 0){return true;}
	return false;
}

Boolean inFunction(){
	if (getloop() == 0){ return true;}
	return false;
}

uint32_t getloop(){
	return stackloop->num;
}

void setloop(uint32_t loop){
	loopcounter = loop;
}

void resetloop(){
	loopcounter = 0;
}

void enterloop(){
	++loopcounter;
}

void exitloop(){
	--loopcounter;
}

/*******************		REST	**********************/
void yyerror (char* yaccProvidedMessage){
	fprintf(stderr, "%s: at line %d, before token: %s\n", yaccProvidedMessage, yylineno, yytext);
}


uint32_t powerof(uint32_t x){
	int power = 1;
	while(x>0){
		power = power*10;
		x--;
	}
	return power;
}

char* nonameFunc(){
	uint32_t num_digits=0, num, remaining, digit=0, index=1, dia;
	char* str=malloc(5);

	FATAL_ERROR(str, "Cant allocate memory for new nonamefunc");

	str[0] = '$';
	if(!fnn){
		str[1] = '0';
		str[2] = '\0';
		fnn++;	
		return str;
	}
	num=fnn;
	remaining=fnn;
	while(num>0){
		num=num/10;
		num_digits++;
	}
	dia = powerof(num_digits);
	while(num_digits>0){
		digit = remaining / powerof(num_digits-1);
		remaining = remaining % powerof(num_digits-1);
		str[index] = 48 + digit;
		num_digits--;
		index++;
	}
	str[4] = '\0';
	fnn++;
	return str;
}



/*******************	SYMBOL TABLE	*********************/


uint32_t  hash( const char* name){
	uint32_t i, sum = 0;

	for(i=0; i<strlen(name); i++){
		sum = sum + (uint32_t)name[i];
	}
        return sum % MAX_BUCKETS;
}

Boolean SymbolTable_create(void){

	if( !(symtable=malloc(sizeof(struct SymbolTable)))){
		yyerror("ERROR: Can not allocate enough memory for new Symbol Table!");
		return false;
	}else if( !(symtable->hashTable=malloc(sizeof(SymbolTableEntry) * MAX_BUCKETS))){
		yyerror("ERROR: Can not allocate enough memory for new Symbol Table's HashTable!");
		return false;
	}else  if( !(symtable->scopeTable=malloc(sizeof(SymbolTableEntry) * MAX_SCOPES))){
		yyerror("ERROR: Can not allocate enough memory for new Symbol Table's ScopeTable!");
		return false;
	}
	symtable->totalSymbols = 0;
	SymbolTable_init();
	programVarOffset = 0;
	return true;
}

SymbolTableEntry* SymbolTableEntry_create(char* name, uint32_t scope, uint32_t line, Symbol_T type){
	SymbolTableEntry *newEntry;

	FATAL_ERROR(name, "SymbolTableEntry name is NULL");
	if(!(newEntry=malloc(sizeof(SymbolTableEntry)))){
		yyerror("Malloc");
		exit(EXIT_FAILURE);
	}

	newEntry->isActive = true;
	newEntry->name = strdup(name);
	newEntry->space = scopespace_get();
		
	newEntry->scope = scope;
	newEntry->line = line;

	newEntry->type = type;
	newEntry->nextc = NULL;
	newEntry->nexts = NULL;
	return newEntry;
}

void SymbolTable_insert(SymbolTableEntry *entry){
	SymbolTableEntry *tmpc, *tmps;
	uint32_t key = hash(entry->name);
	entry->offset = scopeoffset_get();
	scopeoffset_inc();
	

	/****** INSERT NEW ENTRY IN COLLISION LIST *****/
	tmpc = symtable->hashTable[key];
	if( !(tmpc) ){	/*if collision list is empty*/
		symtable->hashTable[key] = entry;
	}else{
		while(tmpc->nextc){
			tmpc = tmpc->nextc;
		}
		tmpc->nextc = entry;	/*else make newEntry final in collision list*/
	}

	/****** INSERT NEW ENTRY IN SCOPE LIST *****/
	tmps = symtable->scopeTable[entry->scope];
	if( !(tmps) ){	/*if scope list is empty*/
		symtable->scopeTable[entry->scope] = entry;
	}else{		/*else make newEntry first in scope list*/
		symtable->scopeTable[entry->scope] = entry;
		entry->nexts = tmps;
	}
	++(symtable->totalSymbols);
}

void SymbolTable_hide(uint32_t scope) {
	SymbolTableEntry* tmp = symtable->scopeTable[scope];
        while(tmp) {
            	tmp->isActive = false;
            	tmp = tmp->nexts;
        }
}



void SymbolTable_init(void){
	SymbolTable_insert(SymbolTableEntry_create("print",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("input",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("objectmemberkeys",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("objecttotalmembers",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("objectcopy",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("totalarguments",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("argument",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("typeof",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("strtonum",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("sqrt",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("cos",0,0,libraryfunc_s));
	SymbolTable_insert(SymbolTableEntry_create("sin",0,0,libraryfunc_s));
}


void SymbolTable_print(void){
	int len, i,j;
        struct SymbolTableEntry *tmps;
        for(i = 0; i<MAX_SCOPES; i++){
            	len = 0;
            	tmps = symtable->scopeTable[i];
            	if(!tmps) {
                	continue;
            	}
            	fprintf(stdout, "\n-----------\tScope #%d\t-----------\n", i);
            	while(tmps) {
                	len++;
                	tmps = tmps->nexts;
            	}
            	while(len > 0) {
                	tmps = symtable->scopeTable[i];
                	for(j = 0; j < len - 1; j++) {
                 		tmps = tmps->nexts;
                	}	
                	switch(tmps->type){
                    		case 0: fprintf(stdout,"\"%s\"  [%s]  (line %d)  (scope %d)\n",
                   			tmps->name, "program variable", tmps->line, tmps->scope);
                    			break;
                    		case 1: fprintf(stdout,"\"%s\"  [%s]  (line %d)  (scope %d)\n",
                    			tmps->name, "program function", tmps->line, tmps->scope);
                    			break;
                    		case 2: fprintf(stdout,"\"%s\"  [%s]  (line %d)  (scope %d)\n",
                    			tmps->name, "library function", tmps->line, tmps->scope);
                    			break;
			}
                	len--;
            	}
        }
}

SymbolTableEntry* SymbolTable_lookup(char* name, int flag){
		SymbolTableEntry* tmp = NULL;
		int i;
		FATAL_ERROR(name, "Name is NULL");
		access = 0;
		switch(flag){
			case(0):
				for(i=currscope_get(); i>=0; i--){
					tmp = symtable->scopeTable[i];
					while(tmp){
						if(tmp->type >0){access=1;}
						if( !(strcmp(tmp->name,name)) && (tmp->isActive)){
							return tmp;
						}
						tmp = tmp->nexts;
					}
				}
				break;
			case(1):
				tmp = symtable->scopeTable[currscope_get()];
				while(tmp){
					if( !(strcmp(tmp->name,name)) && (tmp->isActive)){
						return tmp;
					}
					tmp = tmp->nexts;
				}
				break;
			case(2):
				tmp = symtable->scopeTable[0];
				while(tmp){
					if( !(strcmp(tmp->name,name)) && (tmp->isActive)){
						return tmp;
					}
					tmp = tmp->nexts;
				}
			}
			return NULL;
		}

SymbolTableEntry* SymbolTable_Clookup(char* name){
	SymbolTableEntry* tmp;
	FATAL_ERROR(name, "Cant lookup NULL name");
	tmp = symtable->hashTable[hash(name)];
	while(tmp){
		if ( (!strcmp(tmp->name, name)) && tmp->isActive){
			return tmp;
		}
		tmp = tmp->nextc;
	}
	return NULL;
}

SymbolTableEntry* SymbolTable_Slookup(char* name, uint32_t scope){
	SymbolTableEntry* tmp;
	FATAL_ERROR(name, "Cant lookup NULL name");
	
	tmp = symtable->scopeTable[scope];
	while(tmp){
		if( !(strcmp(tmp->name,name))){
			return tmp;
		}
		tmp = tmp->nexts;
	}
	return NULL;
}

Boolean lib_func_found(char* name){
	FATAL_ERROR(name, "NULL name");
	if( !(strcmp(name,"print")) || !(strcmp(name,"input")) || !(strcmp(name,"objectmemberkey")) || !(strcmp(name,"objecttotalmembers")) || !(strcmp(name,"objectcopy")) || !(strcmp(name,"totalarguments")) || !(strcmp(name,"argument")) || !(strcmp(name,"typeof")) || !(strcmp(name,"sqrt")) || !(strcmp(name,"strtonum")) || !(strcmp(name,"cos")) || !(strcmp(name,"sin")) ){ 
			return true;
		}
	return false;
}

Boolean symisFunc(SymbolTableEntry* entry){
	if(entry){
		if (entry->type > 0){ 
			return true;
		}
	}
	return false;	
}

Boolean exprisFunc(Expression* expr){
	if(expr){
		if( (expr->type == programfunc_e) || (expr->type == libraryfunc_e)){
			return true;
		}
	}
	return false;
}



