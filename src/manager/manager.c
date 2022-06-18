/*	File: manager.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation
 */

#include "manager.h"


/***************	stmts	*********************/	
Statement* Manager_stmt(Statement* stmt){
	return stmt;
}

Statement* Manager_stmts(Statement* stmt1, Statement* stmt2){
	return merge(stmt1,stmt2);
}

/***************	stmt	*********************/	
Statement* Manager_stmt_expr(Expression* expr){
	if(expr){
		if(expr->type == 5){
			expr_eval(expr);	/* make boolexpr_e expressions, true or false */
		}
	}
	return NULL;
}

Statement* Manager_stmt_stmt(Statement* stmt){
	return stmt;
}

Statement* Manager_stmt_null(){
	return NULL;
}

Statement* Manager_break(){
	FATAL_ERROR(inLoop(), "Cant use break out of a loop");
	return new_stmt(1);
}

Statement* Manager_continue(void){
	FATAL_ERROR(inLoop(), "Cant use continue out of a loop");
	return new_stmt(2);
}

/****************	expr		****************/
Expression* Manager_expr_assignexpr(Expression* assign){
	return assign;
}

Expression* Manager_expr_term(Expression* term){
	return term;
}

Expression* Manager_expr_arithop(Expression* expr1, Iopcode op, Expression* expr2){
	Expression* result;
	FATAL_ERROR(expr1,"Cant do arithmetic operation with NULL expr");	
	FATAL_ERROR((check_arith(expr1)),"Cant do arithmetic operation with NULL expr");
	FATAL_ERROR(expr2,"Cant do arithmetic operation with NULL expr");
	FATAL_ERROR((check_arith(expr2)),"Cant do arithmetic operation with NULL expr");
	
	result = newexpr(arithexpr_e);
	result->sym = newtemp();
	emit(op, expr1 , expr2, result, 0 , yylineno);
	return result;
}

Expression* Manager_expr_relop(Expression* expr1, Iopcode op, Expression* expr2){
	Expression* result = newexpr(boolexpr_e);
	result->sym = newtemp();
	result->tlist = new_uint(nextquadlabel());
       	result->flist = new_uint(nextquadlabel()+1);
	emit(op,expr1,expr2,NULL,0,yylineno);
	emit(jump,NULL,NULL,NULL, 0, yylineno);
	return result;	
}


Expression* Manager_expr_boolop(Expression* expr1, Iopcode op, uint32_t m, Expression* expr2){
	Expression* result = newexpr(boolexpr_e);
	result->sym = newtemp();
	Manage_check_expr(expr2);
		
	switch (op){
		case or:
			backpatch(expr1->flist,m);
			result->tlist = merge_tlist(expr1,expr2);
			result->flist = expr2->flist;
			break;
		case and:
			backpatch(expr1->tlist,m);
			result->tlist = expr2->tlist;
			result->flist = merge_flist(expr1,expr2);
			break;
		default:
			FATAL_ERROR(0,"DEFAULT");
	}

	return result;
}

void Manage_check_expr(Expression* expr){
	FATAL_ERROR(expr,"Cant check NULL expression");
	if( !expr->tlist ){
		make_tflist(expr);
	}
}

/***************	term	*********************/
Expression* Manager_term_expr(Expression* expr){
	return expr;
}

Expression* Manager_term_minusexpr(Expression* expr){
	Expression* result;
	FATAL_ERROR((check_arith(expr)),"Cant do minus expr with NULL expr");
	result = newexpr(arithexpr_e);
	result->sym = newtemp();
	emit(uminus,expr,NULL,result, 0, yylineno);
	return result;
}

Expression* Manager_term_notexpr(Expression* expr){
	FATAL_ERROR(expr,"Cant use NULL expression to not");
	Expression* result = newexpr(boolexpr_e);
	result->sym = newtemp();
	
	if(!expr->tlist){	
		make_tflist(expr);	
	}
	result->tlist = expr->flist;
	result->flist = expr->tlist;

	return result;

}

Expression* Manager_term_plus2lvalue(Expression* lvalue){
	Expression* result;
	FATAL_ERROR((check_arith(lvalue)),"Cant do ++ with NULL expression");
	if(lvalue->type == tableitem_e){
		result = emit_iftableitem(lvalue);
		emit(add,result,newexpr_constnum(1),result, 0 , yylineno);
		emit(tablesetelem, lvalue, lvalue->index, result, 0 , yylineno);
	}else{
		emit(add, lvalue, newexpr_constnum(1), lvalue, 0 , yylineno);
		result = newexpr(arithexpr_e);
		result->sym = newtemp();
		emit(assign, lvalue, NULL, result, 0 , yylineno);
	}
	return result;
}

Expression* Manager_term_lvalueplus2(Expression* lvalue){
	Expression *result, *val;
	FATAL_ERROR((check_arith(lvalue)),"Cant do ++ with NULL expression");
	result = newexpr(var_e);
	result->sym = newtemp();
	if(lvalue->type == tableitem_e){
		val = emit_iftableitem(lvalue);
		emit(assign, val, NULL, result, 0 , yylineno);
		emit(add, val, newexpr_constnum(1), val, 0 , yylineno);
		emit(tablesetelem, lvalue, lvalue->index,val, 0, yylineno);
	}else{
		emit(assign, result, lvalue, NULL, 0 , yylineno);
		emit(add,lvalue, newexpr_constnum(1), lvalue, 0 , yylineno);
	}
	return result;
}

Expression* Manager_term_minus2lvalue(Expression* lvalue){
	Expression* result;	
	FATAL_ERROR((check_arith(lvalue)),"Cant do ++ with NULL expression");
	if(lvalue->type == tableitem_e){
		result = emit_iftableitem(lvalue);
		emit(sub,result,newexpr_constnum(1),result, 0 , yylineno);
		emit(tablesetelem, lvalue, lvalue->index, result, 0 , yylineno);
	}else{
		emit(sub, lvalue, newexpr_constnum(1), lvalue, 0 , yylineno);
		result = newexpr(arithexpr_e);
		result->sym = newtemp();
		emit(assign, lvalue, NULL, result, 0 , yylineno);
	}
	return result;

}

Expression* Manager_term_lvalueminus2(Expression* lvalue){
	Expression *result, *val;
	FATAL_ERROR((check_arith(lvalue)),"Cant do -- with NULL expression");
	result = newexpr(var_e);
	result->sym = newtemp();
	if(lvalue->type == tableitem_e){
		val = emit_iftableitem(lvalue);
		emit(assign, val, NULL, result, 0 , yylineno);
		emit(sub, val, newexpr_constnum(1), val, 0 , yylineno);
		emit(tablesetelem, lvalue, lvalue->index,val, 0, yylineno);
	}else{
		emit(assign, result, lvalue, NULL, 0 , yylineno);
		emit(sub,lvalue, newexpr_constnum(1), lvalue, 0 , yylineno);
	}
	return result;
}

Expression* Manager_term_primary(Expression* primary){
	return primary;
}


/***************	assignexpr   ****************/
Expression* Manager_assignexpr(Expression* lvalue, Expression* expr){
	Expression* result;
	
	if(symisFunc(lvalue->sym)){ FATAL_ERROR(0,"Cant use function as an lvalue");}
	if(exprisFunc(lvalue)){ FATAL_ERROR(0,"Cant use function as an lvalue");}
	if(expr->type == 5){
		expr_eval(expr);	/* make boolexpr_e expression, true or false */
	}

	if(lvalue->type == tableitem_e){
		emit(tablesetelem,lvalue,lvalue->index,expr,0,yylineno);
		result = emit_iftableitem(lvalue);
		result->type = assignexpr_e;
	}else{
		emit(assign, expr, NULL,lvalue, 0 , yylineno);
		result = newexpr(assignexpr_e);
		result->sym = newtemp();
		emit(assign, lvalue, NULL,result, 0 , yylineno);
	}
	return result;
}


/***************	primary	*********************/
Expression* Manager_primary_lvalue(Expression* lvalue){
	return emit_iftableitem(lvalue);
}


Expression* Manager_primary_call(Expression* call){
	return call;
}

Expression* Manager_primary_funcdef(SymbolTableEntry* func){
	Expression* result = newexpr(programfunc_e);
	result->sym = func;
	return result;
}

Expression* Manager_primary_objectdef(Expression* objectdef){
	return objectdef;	
}

Expression* Manager_primary_const(Expression* con){
	return con;
}

/****************	lvalue		****************/
Expression* Manager_lvalue_id( char* name){
	Expression* result = newexpr(var_e);
	lookupEntry = SymbolTable_lookup(name,0);

	if(!lookupEntry){
		lookupEntry = SymbolTableEntry_create(name,currscope_get(),yylineno, var_s);
		SymbolTable_insert(lookupEntry);
	}else{
		if( (lookupEntry->scope != currscope_get()) && (lookupEntry->scope != 0) && (access == 1) ){
			FATAL_ERROR(0,"Cant access variable, out of scope");
		}
		if(lookupEntry->type == 1){
			result->type = programfunc_e;
		}else if(lookupEntry->type == 2){
			result->type = libraryfunc_e;
		}
	}
	result->sym = lookupEntry;
	return result;
}

Expression* Manager_lvalue_localid(char* name){
	Expression* result = newexpr(var_e);
	lookupEntry = SymbolTable_lookup(name,1);
	
	if(lib_func_found(name) && (currscope_get()>0)){
		FATAL_ERROR(0,"Cant use library function name");
	}

	if(!lookupEntry){
		lookupEntry = SymbolTableEntry_create(name,currscope_get(),yylineno, var_s);
		SymbolTable_insert(lookupEntry);

	}	
	result->sym = lookupEntry;
	return result;
}

Expression* Manager_lvalue_col2id(char* name){
	Expression* result = newexpr(var_e);
	lookupEntry = SymbolTable_lookup(name,2);
	
	if(!lookupEntry){
		FATAL_ERROR(0,"Global id not found");
	}
	result->sym = lookupEntry;
	return result;
}

Expression* Manager_lvalue_member(Expression* expr){
	FATAL_ERROR(expr, "Passing NULL member to lvalue");
	return expr;
}

/***************	member	*********************/
Expression* Manager_member_lvalueid(Expression* lvalue, char* name){
	return member_item(lvalue,name);
}

Expression* Manager_member_lvalueexpr(Expression* lvalue, Expression* expr){
	FATAL_ERROR(expr, "Cant create Expression from NULL expr");
	Expression* result = newexpr(tableitem_e);
	lvalue = emit_iftableitem(lvalue);
	result->sym = lvalue->sym;
	result->index = expr;
	return result;
}

Expression* Manager_member_callid(Expression* call, char* name){
	FATAL_ERROR(call, "Cant create Expression from NULL call");
	return member_item(call,name);
}

Expression* Manager_member_callexpr(Expression* call, Expression* expr){
	FATAL_ERROR(call, "Cant create Expression from NULL call");
	FATAL_ERROR(expr, "Cant create Expression from NULL expr");
	Expression* result = newexpr(tableitem_e);
	call = emit_iftableitem(call);
	result->sym = call->sym;
	result->index = expr;
	return result;
}

/***************	call	 *******************/
Expression* Manager_call_callelist(Expression* call, Expression* elist){
	return make_call(call,elist);
}

Expression* Manager_call_lvaluecallsuffix(Expression* lvalue, Call* callsuffix){
     Expression* tmp;
     lvalue = emit_iftableitem(lvalue);
     if(callsuffix->method){
         tmp = lvalue;
         lvalue = emit_iftableitem(member_item(tmp, callsuffix->name));
	 tmp->next = callsuffix->elist;
	 callsuffix->elist->prev=tmp;
	 callsuffix->elist = tmp;
     }
     return make_call(lvalue, callsuffix->elist);
}

Expression* Manager_call_funcdefelist(SymbolTableEntry* funcdef,Expression* elist){
	FATAL_ERROR(funcdef, "Cant call NULL function");
	Expression* func = newexpr(programfunc_e);
	func->sym = funcdef;
	return make_call(func, elist);
}


/***************	callsuffix ******************/
Call* Manager_callsuffix_normcall(Call* normcall){
	return normcall;
}

Call* Manager_callsuffix_methodcall(Call* methodcall){
	return methodcall;
}

/***************	normcall  *******************/
Call* Manager_normcall(Expression* elist){
	Call* result = malloc(sizeof(Call));
	FATAL_ERROR(result, "Cant allocate memory for new Call");
	memset(result, 0 , sizeof(Call));
	result->elist = elist;
	result->method = 0;
	result->name = NULL;
	return result;
}

/***************	methodcall  *****************/
Call* Manager_methodcall(char* name, Expression* elist){
	Call* result = malloc(sizeof(Call));
	FATAL_ERROR(result, "Cant allocate memory for new Call");	
	memset(result, 0 , sizeof(Call));
	result->elist = elist;
	result->method = 1;
	result->name = strdup(name);
	return result;
}

/***************	elist	 ********************/
Expression* Manager_elist_exprs(Expression* exprs){
	return exprs;
}

Expression* Manager_elist_empty(){
	return NULL;
}

/***************	exprs  	*********************/
Expression* Manager_exprs_expr(Expression* expr){
	return expr;
}

Expression* Manager_exprs_exprs(Expression* elist, Expression* expr){
	Expression* tmp = elist;
	while(tmp->next){
		tmp = tmp->next;
	}
	tmp->next = expr;
	expr->prev = tmp;
	return elist;
}	

/***************	objectdef  ******************/
Expression* Manager_objectdef(Expression* opt){
	return opt;
}

/***************	objectdef opts  *************/
Expression* Manager_objectdef_elist(Expression* elist){
	int i;
	Expression* result = newexpr(newtable_e);
	Expression* tmp = elist;
	result->sym = newtemp();
	emit(tablecreate,NULL,NULL,result,0,yylineno);
	for(i=0; tmp; i++){	
		emit(tablesetelem, result, newexpr_constnum(i), tmp, 0 , yylineno);
		tmp = tmp->next;
	}
	return result;
}

Expression* Manager_objectdef_indexed(Indexed* indexed){
	Expression* result = newexpr(newtable_e);
	Indexed* tmp = indexed;
	result->sym = newtemp();
	emit(tablecreate, NULL, NULL, result, 0, yylineno);
	while(tmp){
		emit(tablesetelem, result, tmp->key, tmp->value,0,yylineno);
		tmp = tmp->next;
	}
	return result;
}

/***************	indexed  ********************/
Indexed* Manager_indexed(Indexed* indexed){
	return indexed;
}

/***************	indexedelems  ***************/
Indexed* Manager_indexed_element(Indexed* indexed){
	return indexed;
}

Indexed* Manager_indexed_elements(Indexed* indexed1, Indexed* indexed2){
	Indexed* tmp = indexed1;
	while(tmp->next){
		tmp = tmp->next;
	}
	tmp->next = indexed2;
	
	return indexed1;
}

/***************	indexedelem   ***************/
Indexed* Manager_indexedelem(Expression* key, Expression* value){
	FATAL_ERROR(key, "Cant have NULL key in indexed element");
	FATAL_ERROR(value, "Cant have NULL value in indexed element");
	Indexed *result = malloc(sizeof(Indexed));
	FATAL_ERROR(result, "Cant allocate enough memory for new indexed element");
	memset(result, 0 , sizeof(Indexed));
	result->key = key;
       	result->value = value;
	result->next = NULL;
	return result;
}

/***************	funcdef	*********************/
char* Manager_funcname_id( char* name){
	if(lib_func_found(name)){
		FATAL_ERROR(0,"Cant define function with library function name");	
	}

	if(SymbolTable_lookup(name,1)){
		FATAL_ERROR(0,"Function name reserved in curresnt scope");
	}
	return strdup(name);
}

char* Manager_funcname_empty(){
	return nonameFunc();
}

SymbolTableEntry* Manager_funcprefix(char* name){
	SymbolTableEntry* result = SymbolTableEntry_create(name, currscope_get(), yylineno, programfunc_s);
	result->iaddress = nextquadlabel();
	SymbolTable_insert(result);
	emit(funcstart,NULL,NULL,lvalue_expr(result),0,yylineno);
	push_offset(scopeoffset_get());
	scopespace_inc();
	formalargoffset_reset();
	currscope_inc();
	return result;
}

void Manager_funcargs(){
	scopespace_inc();
	functionlocaloffset_reset();
	currscope_dic();
}

void Manager_funcblockstart(void){
	push_loop(0);
}

void Manager_funcblockend(void){
	pop_loop();
	setloop(stackloop->num);
}


Statement* Manager_funcbody(Statement* block){
	Statement* tmp, *tmp2;
	tmp = malloc(sizeof(Statement));
	tmp2 = malloc(sizeof(Statement));
	Uint* total = malloc(sizeof(Uint));
	FATAL_ERROR(total, "Cant allocate enough memory for new Uint");
	memset(total,0,sizeof(Uint));
	total->num = scopeoffset_get();
	if(block){	
		total->next = block->rlist;
		block->rlist = total;	
		return block;
	}else{
		tmp->rlist = total;
		tmp2 = new_stmt(3);
		tmp->rlist->next = tmp2->rlist;
		return tmp;
	}

	scopespace_dic();
}

SymbolTableEntry* Manager_funcdef( uint32_t n,SymbolTableEntry* prefix, Statement* body){
	uint32_t offset;
	scopespace_dic();
	scopespace_dic();
	prefix->totalLocals = body->rlist->num;
	body->rlist = body->rlist->next;
	offset = pop_offset();
	scopeoffset_restore(offset);
	emit(funcend, NULL, NULL,  lvalue_expr(prefix),0 , yylineno);
	patchlabel(n,nextquadlabel());
	patch_rlist(body,nextquadlabel()-1);
	return prefix;
}

/****************	block		****************/
void Manager_block_open(){
	currscope_inc();
}

Statement* Manager_block_close(Statement* stmt){
	SymbolTable_hide(currscope_get());
	currscope_dic();
	return stmt;
}



/****************	const		****************/
Expression* Manager_const_int(int num){
	return newexpr_constnum(num);
}

Expression* Manager_const_real(double num){
	return newexpr_constnum(num);
}

Expression* Manager_const_string(char* str){
	return newexpr_conststring(str);
}

Expression* Manager_const_nil(){
	return  newexpr(nil_e);
}

Expression* Manager_const_boolean(Boolean b){
	return newexpr_constbool(b);
}


/***************	idlist	*********************/
void Manager_idlist(char* name){
	if(lib_func_found(name)){
		FATAL_ERROR(0,"Cant use lib func as argument");	
	}

	if(SymbolTable_lookup(name,1)){
		FATAL_ERROR(0,"Cant use same argument twice");
	}
	
	SymbolTable_insert(SymbolTableEntry_create(name, currscope_get(),yylineno, var_s));
}

/***************	ifprefix  *******************/
Expression* Manager_ifprefix(Expression* expr){
	if(expr){
		if(expr->tlist || expr->flist){
			backpatch(expr->tlist, nextquadlabel());
		}else{
			expr->flist = new_uint(nextquadlabel()+2);
			emit(if_eq,expr,newexpr_constbool(1),NULL, nextquadlabel()+2, yylineno);
			emit(jump,NULL,NULL,NULL,0,yylineno);
		}
	}
	
		return expr;
}

/***************	ifstmt	*********************/
Statement* Manager_if(Expression* ifprefix, Statement* stmt){
	if(ifprefix){
		backpatch(ifprefix->flist, nextquadlabel());
	}
	return stmt;
}

Statement* Manager_ifelse(Expression* ifprefix, Statement* stmt1, uint32_t n, uint32_t m, Statement* stmt2){
	if(ifprefix){
		patchlabel(n, nextquadlabel());
		backpatch(ifprefix->flist, m);
	}
	return merge(stmt1, stmt2);
}

/***************	loop ******************/
void Manager_loopstart(){
	++(stackloop->num);	

}

void Manager_loopend(){
	--(stackloop->num);

}

Statement* Manager_loopstmt(Statement* stmt){
	return stmt;
}


/***************	whilestart ******************/
uint32_t Manager_whilestart(void){
	return nextquadlabel();
}

/***************	whilecond   *****************/
Expression* Manager_whilecond(Expression* expr){
	
	if(expr){
		if (expr->tlist || expr->flist){
			backpatch(expr->tlist, nextquadlabel());
		}else{
			expr->flist = new_uint(nextquadlabel()+1);
			emit(if_eq, expr, newexpr_constbool(1), NULL, nextquadlabel()+2, yylineno);
			emit(jump,NULL,NULL,NULL,0 ,yylineno);
	
		}
	}
	return expr;
}

/***************	while	*********************/
void Manager_while(uint32_t whilestart, Expression* whilecond, Statement* stmt){
	emit(jump,NULL,NULL,NULL,whilestart, yylineno);
	backpatch(whilecond->flist,nextquadlabel());
	if(stmt){
		patch_blist(stmt,nextquadlabel());
		patch_clist(stmt, whilestart);
	}
}

/***************	M N	*********************/
uint32_t Manager_n(void){
	uint32_t result = nextquadlabel();
	emit(jump, NULL, NULL, NULL, 0, yylineno);
	return result;
}

uint32_t Manager_m(void){
	return nextquadlabel();
}

/***************	forprefix	*************/
Forprefix* Manager_forprefix(uint32_t m, Expression* expr){
	Forprefix* result = malloc(sizeof(Forprefix));
	FATAL_ERROR(result, "Cant allocate enough memory for new forprefix");
	memset(result,0,sizeof(Forprefix));
	result->test = m;
	result->enter = nextquadlabel();
	emit(if_eq, expr, newexpr_constbool(1), NULL, 0, yylineno);
	return result;
}

/***************	for	*********************/
void Manager_for(Forprefix* forprefix, uint32_t n1, uint32_t n2, Statement* stmt, uint32_t n3){
	patchlabel(forprefix->enter, n2+1);
	patchlabel(n1, nextquadlabel());
	patchlabel(n2, forprefix->test);
	patchlabel(n3, n1+1);
	patch_blist(stmt,nextquadlabel());
	patch_clist(stmt,n1+1);
}

/***************	return	*********************/
Statement* Manager_return(){
	FATAL_ERROR(inFunction(),"Cant call return out of a function");
	emit(ret, NULL,NULL,NULL,0,yylineno);
	return new_stmt(3);
}

Statement* Manager_return_expr(Expression* expr){
	FATAL_ERROR(inFunction(),"Cant call return out of a function");
	emit(ret, NULL, NULL,expr,0,yylineno);
	return new_stmt(3);
}
