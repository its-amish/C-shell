#include"../include/shell.h"
#include"../include/ping.h"
#include"../include/linkedlist.h"

void ping(char** args){
    if(!args[1]||!args[2]){
        printf("Invalid Syntax!");
    }

    // also we need to check that 

    /// LLM GENERATED COD STARTS////

    for (char* c = args[2]; *c; c++) {
        if (!isdigit(*c)) {
            printf("Invalid syntax!\n");
            return;
        }
    }
    
    ////LLM GENERATED CODE ENDS////

    pid_t pid=atoi(args[1]);
    int signal=atoi(args[2]);
   int actual_signal=signal%32;


    if(kill(pid,actual_signal)==0){
        printf("Sent signal %d to process with pid %d\n",signal,pid);
    }else{
        if(errno==ESRCH){
            printf("No such process found\n");
        }

    }
}