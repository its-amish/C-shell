#ifndef LINKEDLIST
#define LINKEDLIST
#include"shell.h"

typedef struct node{
    pid_t pid;
    char name[1000];
    int job_number;
    char status;
    struct node* next;
}node;

node* find_job(pid_t pid);
void cleanjobs();
node* find_job_number(int n);

background_job* remove_job(pid_t pid,background_job* head);
background_job* find_recent(background_job* head);
background_job* find_no(background_job* head,int no);
background_job* find_pid(background_job* head,pid_t pid);
background_job* add(background_job* head,pid_t pid,char* command_name,char state,pid_t pgid);
#endif