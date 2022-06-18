/*	File: lex.h
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: ADT and Interface
 */		

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct commNode{
    unsigned int startline ;
    unsigned int endline;
    int closed;
    struct commNode *next;
    struct commNode *prev;
};

struct commNode *commHead;

struct commNode *commTail;

void openComm(unsigned int line);

void closeComm(unsigned int line);



char* copyStr( char* buffer);











