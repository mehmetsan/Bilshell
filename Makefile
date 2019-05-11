all: bilshell 
cost: bilshell.c 
	gcc -Wall -g -o bilshell bilshell.c 
clean: rm -fr bilshell bilshell.o *~
