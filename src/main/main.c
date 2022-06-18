#include "main_functions.h"



int main(int argc, char**argv){
	int read = binaryfile_read(argv[1]);
	if(read){
		avm_textfile();
		avm_initialize();
		while(!executionFinished){
			execute_cycle();
		}
	}
	return 0;
}

int binaryfile_read(char* file_name){
	uint32_t mn, i;
	FILE* file = fopen(file_name, "rb");
	if(!file){
		fprintf(stderr, "Cant open file %s\n", file_name);	
		return 0;
	}
	
	fread(&mn, sizeof(uint32_t), 1, file);

	if(mn!=340200501){
		fprintf(stderr,"Wrong magic number\n");
		return 0;
	}
	binaryarrays_read(file);
	binaryinstr_read(file);
	fread(&i, sizeof(uint32_t), 1,file);
	
	top = AVM_STACKSIZE - i -1;
	topsp = AVM_STACKSIZE -1;
	fclose(file);
	return 1;
}

void binaryarrays_read(FILE* file){
	uint32_t i=0, len, addr, localsize, ilen, llen;
	char *str, *id, *name;
	double num;
	fread(&stringConsts_curr, sizeof(uint32_t), 1,file);
	stringConsts = malloc(stringConsts_curr * sizeof(char*));
	while( i<stringConsts_curr){
		fread(&len, sizeof(uint32_t), 1, file);
		str = malloc(len);
		fread(str, sizeof(char), len, file);
		stringConsts[i] = str;
		++i;
	}

	fread(&numConsts_curr, sizeof(uint32_t), 1,file);
	numConsts = malloc( numConsts_curr * sizeof(double));
	i=0;
	while(i<numConsts_curr){
		fread(&num, sizeof(double), 1, file);
		numConsts[i] = num;
		++i;
	}

	fread(&userFuncs_curr,sizeof(uint32_t), 1, file);
	userFuncs = malloc(userFuncs_curr * sizeof(Userfunc));
	i=0;
	while(i<userFuncs_curr){
		fread(&addr, sizeof(uint32_t), 1 , file);
		fread(&localsize, sizeof(uint32_t),1,file);
		fread(&ilen, sizeof(uint32_t), 1, file);
		id = malloc(ilen+1);
		fread(id, sizeof(char), ilen, file);
		id[ilen] = '\0';
		userFuncs[i].localSize = localsize;
		userFuncs[i].address = addr;
		userFuncs[i].id = id;
		++i;
	}

	i=0;
	fread(&namedLibfuncs_curr, sizeof(uint32_t), 1, file);
	namedLibfuncs = malloc(namedLibfuncs_curr * sizeof(char));
	while(i<namedLibfuncs_curr){
		fread(&llen, sizeof(uint32_t), 1, file);
		name = malloc(llen+1);
		fread(name, sizeof(char), llen, file);
		name[llen] = '\0';
		namedLibfuncs[i] = name;
		++i;
	}
}


void binaryinstr_read(FILE* file){
	uint32_t i=0,  rv, a1v, a2v, line;
	char	op,rt,a1t,a2t;
	fread(&codeSize, sizeof(uint32_t), 1, file);
	instructions = malloc(codeSize * sizeof(Instruction));
	while(i<codeSize){
		fread(&op, sizeof(char), 1 , file);
		instructions[i].opcode = op;
		
		fread(&rt, sizeof(char), 1 , file);
		instructions[i].result.type = rt;
		
		fread(&rv, sizeof(uint32_t), 1 , file);
		instructions[i].result.val = rv;
		
		fread(&a1t, sizeof(char), 1 , file);
		instructions[i].arg1.type = a1t;
		
		fread(&a1v, sizeof(uint32_t), 1 , file);
		instructions[i].arg1.val = a1v;
		
		fread(&a2t, sizeof(char), 1 , file);
		instructions[i].arg2.type = a2t;
		
		fread(&a2v, sizeof(uint32_t), 1 , file);
		instructions[i].arg2.val = a2v;
		
		fread(&line, sizeof(uint32_t), 1, file);
		instructions[i].srcLine = line;

		++i;
	}
}


/*** TEXT FILE ***/
void avm_textfile(){
	FILE* file = fopen("avm_textout.txt", "w+");
	if(!file){ FATAL_ERROR(0,"Cant open text file");}

	fprintf(file, "340200501\n");
	avm_textarrays(file);
	avm_textinstr(file);
	fclose(file);
}

void avm_textarrays(FILE* file){
	uint32_t i=0;
	fprintf(file,"\nSTRING CONSTS\n");
	while(i<stringConsts_curr){
		fprintf(file,"%u:\t%s\n", i,stringConsts[i]);
		i++;
	}
	i=0;
	fprintf(file,"\nNUM CONSTS\n");
	while(i<numConsts_curr){
		fprintf(file,"%u:\t%f\n", i,numConsts[i]);
		i++;
	}
	i=0;
	fprintf(file,"\nUSER FUNCTIONS\n");
	while(i<userFuncs_curr){
		fprintf(file,"%u:\tAddress:%u\tLocal Size:%u\tId:%s\n",
				i,userFuncs[i].address, userFuncs[i].localSize, userFuncs[i].id);
		i++;
	}

	i=0;
	fprintf(file,"\nLIBRARY FUNCTIONS\n");
	while(i<namedLibfuncs_curr){
		fprintf(file,"%u:\t%s\n", i,namedLibfuncs[i]);
		i++;
	}
}

void avm_textinstr(FILE* file){
	Instruction tmp;
	uint32_t i = 0;
	fprintf(file,"\nInstrNo\t\tOpcode\t\tResult\t\tArg1\t\tArg2\t\tLine\n");
	while(i<codeSize){
		tmp = instructions[i];
		fprintf(file,"%u\t\t", i);
		avm_textop((uint32_t)tmp.opcode, file);
		if(tmp.result.type==7){ fprintf(file,"\t\t\t");}
		else if(tmp.result.type==10){fprintf(file,"%u\t\t",(uint32_t)tmp.result.type);}
		else{fprintf(file,"%u,%u\t\t",(uint32_t)tmp.result.type, tmp.result.val);}

		if(tmp.arg1.type==7){ fprintf(file,"\t\t\t");}
		else if(tmp.arg1.type==10){fprintf(file,"%u\t\t",(uint32_t)tmp.arg1.type);}
		else{fprintf(file,"%u,%u\t\t",(uint32_t)tmp.arg1.type, tmp.arg1.val);}

		if(tmp.arg2.type==7){ fprintf(file,"\t\t\t");}
		else if(tmp.arg2.type==10){fprintf(file,"%u\t\t",(uint32_t)tmp.arg2.type);}
		else{fprintf(file,"%u,%u\t\t",(uint32_t)tmp.arg2.type, tmp.arg2.val);}

		fprintf(file,"%u", tmp.srcLine);
		fprintf(file,"\n");
		i++;
	}
}

void avm_textop(uint32_t i, FILE* file){
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

