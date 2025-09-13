#include"../include/shell.h"

background_job* add(background_job* head,pid_t pid,char* command_name,char state,pid_t pgid){
    background_job* temp=(background_job*)malloc(sizeof(background_job));
    if(temp==NULL){
        printf("some error came dont know jackshit tho\n");
        return head;
    }

    // strcpy(temp->command,command_name);
    strncpy(temp->command, command_name, sizeof(temp->command) - 1);
    temp->command[sizeof(temp->command)-1]='\0';
    temp->pid=pid;
    temp->next=NULL;
    temp->job_id=no_jobs_background++;
    // strncpy(temp->command, command_name, sizeof(temp->command) - 1);
    temp->command[sizeof(temp->command)-1]='\0';
    temp->state=state;
    temp->pgid=pgid;

    if(head==NULL){
        return temp;
    }


    background_job* temp2=head;
    while(temp2->next!=NULL){
        temp2=temp2->next;
    }
    temp2->next=temp;
    return head;

}

background_job* find_pid(background_job* head,pid_t pid){

    if(head==NULL){
        return NULL;
    }

    while(head!=NULL){
        if(head->pid==pid){
            return head;
        }
        head=head->next;
    }
    return NULL;
}


background_job* find_no(background_job* head,int no){

    if(head==NULL){
        return NULL;
    }

    while(head!=NULL){
        if(head->job_id==no){
            return head;
        }
        head=head->next;
    }
    return NULL;
}


background_job* find_recent(background_job* head){

    if(head==NULL){
        return NULL;
    }

    while(head->next!=NULL){
        head=head->next;
    }
    return head;
}

background_job* remove_job(pid_t pid,background_job* head) {
    background_job* current =head;
    background_job* prev = NULL;

    while (current != NULL && current->pid != pid) {
        prev = current;
        current = current->next;
    }

    // If job wasn't found
    if (current == NULL) {
        return head;
    }

    // Remove node from list
    if (prev == NULL) {
        // Job to remove is the head node
        head = current->next;
    } else {
        // Bypass the current node
        prev->next = current->next;
    }

    // Free memory
    free(current);
    return head;
}