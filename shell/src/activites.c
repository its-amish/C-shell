#include"../include/shell.h"
#include"../include/static_run.h"
#include "../include/activites.h"
#include<stdlib.h>



int comp1(const void* a,const void *b){
    background_temp* a1=(background_temp*)a;
    background_temp* b1=(background_temp*)b;

    return strcmp(a1->name,b1->name);
}

// will be using proc for this its so cool

char getstatus(int pid){ // 0- not exist and 1-stopped and 2-runnnig
    // ohk
    // proc/pid/stat
    char proc_path[1000];
    // // will be saving it 
    // strcat(proc_path,"/proc");
    // // we will be // just use sprintf nhi toh bhut lamba jayega
    // strcat(proc_path,)

    sprintf(proc_path,"/proc/%d/stat",pid);
    FILE* file =fopen(proc_path,"r");
    if(file==NULL){
        return '\0'; // means the process doesnt exist 
    }

    char state;
    // its always the third field so ig its just works
    fscanf(file, "%*d %*s %c", &state); // LLM GENERATED LINE 
    fclose(file);
    return state;

}


void activites(){
    
    // ohk first we will sort them
    // printf("hello i came here\n");
    background_temp temp2[1000];
    // int n=10000;
    int index=0;

    background_job* temp=job_head;

    while(temp!=NULL){
        char status=getstatus(temp->pid);
        if(status=='T'){
            strcpy(temp2[index].name,temp->command);
            temp2[index].pid=temp->pid;
            strcpy(temp2[index].status,"Stopped");
            index++;
        }else{
            strcpy(temp2[index].name,temp->command);
            temp2[index].pid=temp->pid;
            strcpy(temp2[index].status,"Running");
            index++;
        }

        temp=temp->next;

    }
    qsort(temp2,index,sizeof(background_temp),comp1); 
    for(int i=0;i<index;i++){
        printf("[%d] : %s - %s\n",temp2[i].pid,temp2[i].name,temp2[i].status);
    }

    fflush(stdout);


}