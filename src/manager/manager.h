/*	File: manager.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface 
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "../funcs/funcs.h"

/***************	stmts	*********************/	
Statement* Manager_stmt(Statement* stmt);
Statement* Manager_stmts(Statement* stmt1, Statement* stmt2);

/***************	stmt	*********************/	
Statement* Manager_stmt_null(void);
Statement* Manager_stmt_expr(Expression* expr);
Statement* Manager_stmt_stmt(Statement* stmt);
Statement* Manager_break(void);
Statement* Manager_continue(void);

/***************	expr	*********************/	
Expression* Manager_expr_assignexpr(Expression* assign);
Expression* Manager_expr_arithop(Expression* expr1, Iopcode op, Expression* expr2);
Expression* Manager_expr_relop(Expression* expr1, Iopcode op, Expression* expr2);
Expression* Manager_expr_boolop(Expression* expr1, Iopcode op, uint32_t m, Expression* expr2);
Expression* Manager_expr_term(Expression* term);
void Manage_check_expr(Expression* expr);

/***************	term	*********************/	
Expression* Manager_term_expr(Expression* expr);
Expression* Manager_term_minusexpr(Expression* expr);
Expression* Manager_term_notexpr(Expression* expr);
Expression* Manager_term_primary(Expression* primary);
Expression* Manager_term_plus2lvalue(Expression* lvalue);
Expression* Manager_term_lvalueplus2(Expression* lvalue);
Expression* Manager_term_minus2lvalue(Expression* lvalue);
Expression* Manager_term_lvalueminus2(Expression* lvalue);

/***************	assignexpr   ****************/
Expression* Manager_assignexpr(Expression* lvalue, Expression* expr);

/***************	primary	*********************/
Expression* Manager_primary_lvalue(Expression* lvalue);
Expression* Manager_primary_call(Expression* call);
Expression* Manager_primary_funcdef(SymbolTableEntry* func);
Expression* Manager_primary_objectdef(Expression* objectdef);
Expression* Manager_primary_const(Expression* con);

/***************	lvalue	*********************/
Expression* Manager_lvalue_id( char* name);
Expression* Manager_lvalue_localid( char* name);
Expression* Manager_lvalue_col2id(char* name);
Expression* Manager_lvalue_member(Expression* expr);

/***************	member	*********************/
Expression* Manager_member_lvalueid(Expression* lvalue, char* name);
Expression* Manager_member_lvalueexpr(Expression* lvalue, Expression* expr); 
Expression* Manager_member_callid(Expression* call, char* name);
Expression* Manager_member_callexpr(Expression* call, Expression* expr); 

/***************	call	 *******************/
Expression* Manager_call_callelist(Expression* call, Expression* elist); 
Expression* Manager_call_lvaluecallsuffix(Expression* lvalue, Call* callsuffix);
Expression* Manager_call_funcdefelist(SymbolTableEntry* funcdef,Expression* elist);

/***************	callsuffix ******************/
Call* Manager_callsuffix_normcall(Call* normcall);
Call* Manager_callsuffix_methodcall(Call* methodcall);

/***************	normcall  *******************/
Call* Manager_normcall(Expression* elist);

/***************	methodcall  *****************/
Call* Manager_methodcall(char* name, Expression* elist);


/***************	elist  	*********************/
Expression* Manager_elist_exprs(Expression* exprs);
Expression* Manager_elist_empty(void);

/***************	exprs  	*********************/
Expression* Manager_exprs_exprs(Expression* elist, Expression* expr);
Expression* Manager_exprs_expr(Expression* expr); 

/***************	objectdef  ******************/
Expression* Manager_objectdef(Expression* opt);

/***************	objectdef opts  *************/
Expression* Manager_objectdef_elist(Expression* elist);
Expression* Manager_objectdef_indexed(Indexed* indexed);

/***************	indexed  ********************/
Indexed* Manager_indexed(Indexed* indexed);

/***************	indexedelems  ***************/
Indexed* Manager_indexed_element(Indexed* indexed);
Indexed* Manager_indexed_elements(Indexed* indexed1, Indexed* indexed2);

/***************	indexedelem   ***************/
Indexed* Manager_indexedelem(Expression* key, Expression* value);

/***************	block	*********************/
void Manager_block_open();
Statement* Manager_block_close(Statement* stmt);

/***************	funcdef	*********************/
char* Manager_funcname_id(char* name);
char* Manager_funcname_empty(void);
SymbolTableEntry* Manager_funcprefix(char* name);
void Manager_funcargs(void);
void Manager_funcblockstart(void);
void Manager_funcblockend(void);
Statement* Manager_funcbody(Statement* body);
SymbolTableEntry* Manager_funcdef(uint32_t n,SymbolTableEntry* prefix, Statement* body);

/***************	const	*********************/
Expression* Manager_const_int(int num);
Expression* Manager_const_real(double num);
Expression* Manager_const_string(char* str);
Expression* Manager_const_nil();
Expression* Manager_const_boolean(Boolean b);


/***************	idlist	*********************/
void Manager_idlist(char* name);

/***************	ifprefix  *******************/
Expression* Manager_ifprefix(Expression* expr);

/***************	ifstmt	*********************/
Statement* Manager_if(Expression* ifprefix, Statement* stmt);
Statement* Manager_ifelse(Expression* ifprefix, Statement* stmt1, uint32_t n , uint32_t m, Statement* stmt2);

/***************	loop ******************/
void Manager_loopstart(void);
void Manager_loopend(void);
Statement* Manager_loopstmt(Statement* stmt);

/***************	whilestart ******************/
uint32_t Manager_whilestart(void);

/***************	whilecond   *****************/
Expression* Manager_whilecond(Expression* expr);

/***************	while	*********************/
void Manager_while(uint32_t whilestart, Expression* whilecond, Statement* stmt);

/***************	M N	*********************/
uint32_t Manager_n(void);
uint32_t Manager_m(void);

/***************	forprefix	*************/
Forprefix* Manager_forprefix(uint32_t m, Expression* expr);

/***************	for	*********************/
void Manager_for(Forprefix* forprefix, uint32_t n1, uint32_t n2, Statement* stmt, uint32_t n3);

/***************	return	*********************/
Statement* Manager_return();
Statement* Manager_return_expr(Expression* expr);

#endif
