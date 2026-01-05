all:
	bison -d parser.y
	flex lexer.l
	gcc -o mycompiler parser.tab.c lex.yy.c ast.c compiler.c -lm
	gcc -o vm vm.c -lm

clean:
	rm -f mycompiler parser.tab.c parser.tab.h lex.yy.c