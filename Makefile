clear:
	-rm -f *.exe *.c *.txt *.output *.h *.abc

scan:
	bison --yacc --defines --output=parser.c src/bison/parser.y -v
	flex --outfile=scanner.c src/lex/scanner.l
	gcc -o calc.exe \
		scanner.c \
		parser.c \
		src/funcs/funcs.c \
	       	src/manager/manager.c \
	       	src/tcode/tcode.c \
	       	src/tcode/tprint.c \
	       	src/generate/generate.c 
	gcc -o avm.exe \
		src/main/main.c \
	       	src/avm/avm.c \
	       	src/avm/execute.c \
	       	src/avm/help.c \
	       	src/avm/libraryfunctions.c -lm 


run:
	./calc.exe < test
	./avm.exe binaryout.abc
	
