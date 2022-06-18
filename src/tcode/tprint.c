/*	File: tprint.c
 *	Authors: Ioannis Xoulidis csd3410
 *		 Stavros Karaxristianidis csd3434
 *	Design: Interface Implementation
 */

#include "tcode.h"


/****** PRINT	******/

/*** BINARY FILE ***/
void binaryfile(){
	uint32_t mn = 340200501; ;
	FILE* file = fopen("binaryout.abc", "wb");
	if(!file){ FATAL_ERROR(0,"Cant open binary file");}

	fwrite(&mn, sizeof(uint32_t), 1, file);
	binaryarrays(file);
	binaryinstr(file);
	fwrite(&programVarOffset, sizeof(uint32_t),1,file);
	fclose(file);
}

void binaryarrays(FILE* file){
	uint32_t i=0, len;
	fwrite(&stringConsts_curr, sizeof(uint32_t), 1,file);
	while( i<stringConsts_next()){
		len = strlen(stringConsts[i]);
		fwrite(&len, sizeof(uint32_t), 1, file);
		fwrite(stringConsts[i], sizeof(char), len, file);
		++i;
	}

	fwrite(&numConsts_curr, sizeof(uint32_t), 1,file);
	fwrite(numConsts, sizeof(double), numConsts_next(), file);

	fwrite(&userFuncs_curr, sizeof(uint32_t), 1, file);
	i=0;
	while(i<userFuncs_next()){
		len = strlen(userFuncs[i].id);
		fwrite(&userFuncs[i].address, sizeof(uint32_t), 1, file);
		fwrite(&userFuncs[i].localSize, sizeof(uint32_t), 1, file);
		
		fwrite(&len, sizeof(uint32_t), 1, file);
		fwrite(userFuncs[i].id, sizeof(char), len, file);
		++i;
	}
	
	i=0;
	fwrite(&namedLibfuncs_curr, sizeof(uint32_t), 1,file);
	while( i<namedLibfuncs_next()){
		len = strlen(namedLibfuncs[i]);
		fwrite(&len, sizeof(uint32_t), 1, file);
		fwrite(namedLibfuncs[i], sizeof(char), len, file);
		++i;
	}

}

void binaryinstr(FILE* file){
	uint32_t i=0;
	fwrite(&instructions_curr, sizeof(uint32_t), 1, file);
	while(i<instructions_next()){
		fwrite(&instructions[i].opcode, sizeof(char), 1, file);
		fwrite(&instructions[i].result.type, sizeof(char), 1, file);
		fwrite(&instructions[i].result.val, sizeof(uint32_t), 1, file);
		fwrite(&instructions[i].arg1.type, sizeof(char), 1, file);
		fwrite(&instructions[i].arg1.val, sizeof(uint32_t), 1, file);
		fwrite(&instructions[i].arg2.type, sizeof(char), 1, file);
		fwrite(&instructions[i].arg2.val, sizeof(uint32_t), 1, file);
		fwrite(&instructions[i].srcLine, sizeof(uint32_t), 1, file);
		++i;
	}
}

/*** TEXT FILE ***/
void textfile(){
	FILE* file = fopen("textout.txt", "w+");
	if(!file){ FATAL_ERROR(0,"Cant open text file");}

	fprintf(file, "340200501\n");
	textarrays(file);
	textinstr(file);
	fclose(file);
}

void textarrays(FILE* file){
	uint32_t i=0;
	fprintf(file,"\nNUM CONSTS\n");
	while(i<numConsts_next()){
		fprintf(file,"%u:\t%f\n", i,numConsts[i]);
		i++;
	}
	i=0;
	fprintf(file,"\nSTRING CONSTS\n");
	while(i<stringConsts_next()){
		fprintf(file,"%u:\t%s\n", i,stringConsts[i]);
		i++;
	}
	
	i=0;
	fprintf(file,"\nUSER FUNCTIONS\n");
	while(i<userFuncs_next()){
		fprintf(file,"%u:\tAddress:%u\tLocal Size:%u\tId:%s\n", 
				i,userFuncs[i].address, userFuncs[i].localSize, userFuncs[i].id);
		i++;
	}
	
	i=0;
	fprintf(file,"\nLIBRARY FUNCTIONS\n");
	while(i<namedLibfuncs_next()){
		fprintf(file,"%u:\t%s\n", i,namedLibfuncs[i]);
		i++;
	}
	
}

void textinstr(FILE* file){
	Instruction tmp;
	uint32_t i = 0;
	fprintf(file,"\nInstrNo\t\tOpcode\t\tResult\t\tArg1\t\tArg2\t\tLine\n\n");
	while(i<instructions_next()){
		tmp = instructions[i];
		fprintf(file,"%u\t\t", i);
		textop((uint32_t)tmp.opcode, file);
		if(tmp.result.type==7){ fprintf(file,"\t\t\t");}
		else if(tmp.result.type==10){fprintf(file,"%u\t\t",(uint32_t)tmp.result.type);}
		else{fprintf(file,"%u,%u\t\t",(uint32_t)tmp.result.type, tmp.result.val);}

		if(tmp.arg1.type==7){ fprintf(file,"\t\t\t");}
		else if(tmp.arg1.type==10){fprintf(file,"%u\t\t",(uint32_t)tmp.arg1.type);}
		else{fprintf(file,"%u,%u\t\t",(uint32_t)tmp.arg1.type, tmp.arg1.val);}

		if(tmp.arg2.type==7){ fprintf(file,"\t\t\t");}
		else if(tmp.arg2.type==10){fprintf(file,"%u\t\t",(uint32_t)tmp.arg2.type);}
		else{fprintf(file,"%u,%u\t\t",(uint32_t)tmp.arg2.type, tmp.arg2.val);}
		
		fprintf(file,"%u",tmp.srcLine);
		fprintf(file,"\n");
		i++;
	}
}

void textop(uint32_t i, FILE* file){
	switch(i){
		case 0: {fprintf(file,"assign\t\t"); break;}
        	case 1: {fprintf(file,"add\t\t"); break;}
        	case 2: {fprintf(file,"sub\t\t"); break;}
        	case 3: {fprintf(file,"mul\t\t"); break;}
        	case 4: {fprintf(file,"div\t\t"); break;}
        	case 5: {fprintf(file,"mod\t\t"); break;}
        	case 6: {fprintf(file,"jump\t"); break;}
        	case 7: {fprintf(file,"jeq\t"); break;}
        	case 8: {fprintf(file,"jne\t\t"); break;}
        	case 9: {fprintf(file,"jlt\t\t"); break;}
        	case 10: {fprintf(file,"jgt\t\t"); break;}
        	case 11: {fprintf(file,"jle\t\t"); break;}
        	case 12: {fprintf(file,"jge\t\t"); break;}
        	case 13: {fprintf(file,"call\t\t"); break;}
        	case 14: {fprintf(file,"pusharg\t\t"); break;}
        	case 15: {fprintf(file,"funcenter\t\t"); break;}
       	 	case 16: {fprintf(file,"funcexit\t\t"); break;}
        	case 17: {fprintf(file,"newtable\t\t"); break;}
        	case 18: {fprintf(file,"tablegetelem\t\t"); break;}
        	case 19: {fprintf(file,"tablesetelem\t"); break;}
        	case 20: {fprintf(file,"nop\t"); break;}
	}
}
