#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   
#include <errno.h>    
#include <sys/wait.h> 
#include <iostream>
#include <stdbool.h>

#define BUFFER_SIZE 	25
#define READ_END	0
#define WRITE_END	1


/* Declarations for getline() */
char *input = NULL;
size_t capline = 0; // Capacity

/* Declaration for strtok() */
int i;
char *token;
char *array[512];
char delimit[2] = {' ','\n'};


/* Print out "MY_SHELL" */
void prompt() {
    printf("billshell : ");

}

/* Divide input line into tokens */
bool tokenize(char *input) {
    bool complex = false;
    int i = 0;
    token = strtok(input, delimit);
    while (token != NULL) {
        if(token[0]=='|') {
            complex = true;
        }

        array[i++] = token; // Add tokens into the array
        token = strtok(NULL, delimit);
    }
    array[i] = NULL;
    return complex;
}


int main(int argc, char* argv[]) {
    int inputSize = atoi(argv[1]);
    //BATCH MODE
    if(argc>2) {
        char* inst[1000] = {NULL};
        char buf[1024];
        FILE *fptr;


        fptr = fopen(argv[2] ,"r");
        if ( fptr )

        {   char buf2[512];
            char *ch2[40000];
            bool test = false;

            while (fgets(buf , sizeof(buf), fptr) != NULL)
            {
                int count = 0;
                int n = 0;
                int m =0;

                buf[strlen(buf) - 1] = '\0';
                for(int j = 0; j< strlen(buf); j++) {
                    if(buf[j] != ' ')
                        m++;
                    else {
                        memcpy( buf2, &buf[n], m);
                        buf2[m] = '\0';
                        test = true;
                        ch2[count++] = buf2;
                        n =  n + m +1;
                        m = 0;
                    }

                    if(test == true && j + 1 == strlen(buf)) {
                        char c2[512];
                        memcpy( c2, &buf[n], m);
                        c2[m] = '\0';
                        ch2[count] = c2;

                    }
                }

                if(test != true)
                    ch2[0] = buf;


                int pid = fork();
                if(pid != 0) {
                    int s;
                    waitpid(pid , &s , 0);
                    for(int g = 0; g< 40000; g++)
                        ch2[g] = NULL;
                }

                else if(pid == 0) {
                    execvp(ch2[0], ch2);
                    exit(0);
                }
            }

        }
    }
    //INTERACTIVE MODE
    else {
        while(1) {
            char write_msg[BUFFER_SIZE];
            char read_msg[BUFFER_SIZE];


            int firstPipe[2];   	 // first pipe
            pipe(firstPipe);
            int pipe2[2];		//second pipe
            pipe(pipe2);
            bool check = false;

            prompt();
            getline(&input, &capline, stdin); // Read the user input
            /* Check if input is empty */
            if(strcmp(input,"\n")==0) {
                perror("Please type in a command " );
                continue;
            }
            check = tokenize(input); // Divide line into tokens
            //printf("%d",check);
            /* Check if input is "q", if yes then exit shell */
            if (strcmp(array[0], "q") == 0) {
                printf("SYSTEM : Shell is exit\n");
                return 0;
            }
            if(check == true) {
                int pid = fork(); 
                if (pid != 0) { 
                    int s;
                    waitpid(-1, &s, 0); 

                    close(firstPipe[WRITE_END]);
                    while(read(firstPipe[READ_END], read_msg, BUFFER_SIZE)!=0) {
                        //close(pipe2[READ_END]);
                        write(pipe2[WRITE_END], write_msg, BUFFER_SIZE+1);

                    }
                    close(firstPipe[READ_END]);

                    //SECOND CHILD
                    int  s2;
                    int pid2 = fork();
                    waitpid(pid2, &s2, 0); 
                    if(pid2 == 0) {
                        close(pipe2[WRITE_END]);
                        dup2(pipe2[READ_END],0);
                        close(pipe2[READ_END]);
                        exit(0);

                    }
                    else {
                        close(pipe2[READ_END]);
                        close(pipe2[WRITE_END]);
                    }

                } else {
                    //close(firstPipe[READ_END]);
                    dup2(firstPipe[WRITE_END],1);
                    close(firstPipe[0]);
                    close(firstPipe[1]);
                    close(pipe2[0]);
                    close(pipe2[1]);
                    execvp(array[0], array);

                }
            }
            else {
                int s;
                int pid = fork(); // Create a new process
                if (pid != 0) { // If not successfully completed

                    waitpid(-1, &s, 0); // Wait for process termination
                } else {
                    if(execvp(array[0], array) == -1) { // If returned -1 => something went wrong! If not then command successfully completed */
                        perror("ERROR"); // Display error message
                        exit(errno);
                    }
                }

            }
        }
    }//mode
}
