#include"../include/shell.h"
#include"../include/execute.h"
#include"../include/lets_go.h"
#include"../include/log.h"
#include"../include/reveal.h"
#include"../include/hop.h"
#include"../include/jobs.h"
#include"../include/static_run.h"
#include"../include/bgfg.h"
#include"../include/ping.h"
#include"../include/linkedlist.h"
#include"../include/activites.h"
#include<sys/wait.h>

// so the the problem is i cant run hop for a child


void execute(struct Parser P){

int no_jobs=0;// this will give the no of jobs grp that it will give
 
static job arr[1024]; // i am assuming the max size to be this and yeah thats it thats all of it
// static is just a keyword to make things working good
jobs_differ(P,arr,&no_jobs);

    int saved_stdin=dup(STDIN_FILENO);
    int saved_stdout=dup(STDOUT_FILENO);

for(int i=0;i<no_jobs;i++){
    static cmd_grp arr_cmds[1024];
    int no_cmds=0;
    letsdoit(arr[i].cmds,arr[i].num_cmds,arr_cmds,&no_cmds);// breaked acc to | 

    if(strcmp("fg",arr_cmds[0].arguments[0])==0){
        fg(arr_cmds[0].arguments);
        continue;
    }else if(strcmp("bg",arr_cmds[0].arguments[0])==0){
        bg(arr_cmds[0].arguments);
        continue;
    }else if(strcmp("activites",arr_cmds[0].arguments[0])==0){
        activites();
        continue;
    }else if(strcmp("ping",arr_cmds[0].arguments[0])==0){
        ping(arr_cmds[0].arguments);
        continue;
    }else if(strcmp("log",arr_cmds[0].arguments[0])==0){
        // CRITICAL: log must run in parent to modify history array
        // Handle I/O redirection if needed
        int original_stdin = dup(STDIN_FILENO);
        int original_stdout = dup(STDOUT_FILENO);
        
        if (arr_cmds[0].input_fd != STDIN_FILENO) {
            dup2(arr_cmds[0].input_fd, STDIN_FILENO);
            close(arr_cmds[0].input_fd);
        }
        if (arr_cmds[0].output_fd != STDOUT_FILENO) {
            dup2(arr_cmds[0].output_fd, STDOUT_FILENO);
            close(arr_cmds[0].output_fd);
        }
        
        logs(arr_cmds[0].arguments, arr_cmds[0].no_arguments);
        
        // Restore original stdin/stdout
        dup2(original_stdin, STDIN_FILENO);
        dup2(original_stdout, STDOUT_FILENO);
        close(original_stdin);
        close(original_stdout);
        continue;
    }



    if(strcmp("hop",arr_cmds[0].arguments[0])==0){ 
        execute_pipeline(arr_cmds,no_cmds,0,-1);
        continue;
    }

    int pid_pipe[2];
    if (pipe(pid_pipe) == -1) {
        perror("pipe");
        continue;
    }


    // we wont fork for normal commmands ye
        
        int pid=fork(); // we will say just fork the 
        if(pid<0){
            printf("need to fix something dont know what\n");
        }else if(pid==0){
            // this is child


            // LLM GENERATED CODE STARTS ////

                 // --- CHILD PROCESS ---

        // Create a new process group for this job. PGID = PID.
        close(pid_pipe[0]);
        setpgid(0, 0);

        // Reset signal handlers for the child process to default behavior
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);

        // LLM GENEREATED CODE ENDS//



            if(arr[i].delimiter=='&'){
                // background_name[no_jobs_background]=arr_cmds[i];
                // background_jobs[no_jobs_background++]=getpid();
                execute_pipeline(arr_cmds,no_cmds,1,pid_pipe[1]); // last one is for background like it is there or not
            }else{
                execute_pipeline(arr_cmds,no_cmds,0,-1); 
            }
            exit(0);
    
    
        }else{
            // we will only wait if it is what do we say 

            close(pid_pipe[1]);

            if(arr[i].delimiter!='&'){
                // ths thing that
            // LLM GENERATED CODE STARTS ///

            current_foreground = pid; // Track foreground process group ID

            // Give terminal control to the child process group
            tcsetpgrp(STDIN_FILENO, pid);

            // Wait for the foreground child process to complete
            int status;
            waitpid(pid, &status, WUNTRACED); // Use WUNTRACED to catch stops (Ctrl+Z)

            if (WIFSTOPPED(status)) {
                char full_command[1024] = "";
                for (int k = 0; k < arr[i].num_cmds; k++) {
                    strcat(full_command, arr[i].cmds[k]);
                    if (k < arr[i].num_cmds - 1) strcat(full_command, " ");
                }
                    
                // Add job to list with state 'T' for Stopped
                job_head = add(job_head, pid, full_command, 'T', pid);
                    
                // Print the required message
                printf("\n[%d] Stopped %s\n", no_jobs_background, full_command);
            }
                
            



            // Take back terminal control for the shell
            tcsetpgrp(STDIN_FILENO, shell_pgid);
            current_foreground = 0; // Reset foreground tracker

            // LLM GENERATED CODE ENDS/////



            }else{          
                // and background processes are also there

                /// PROBLEM IS ONLY SAVING THE FIRST STRING IN OF THE LINE ///
                // background_name[no_jobs_background] = strdup(arr_cmds[0].arguments[0]);
                // LLM GENERATED CODE STARTS ///
                pid_t actual_pid;
                // Read the PID of the real command from the pipe
                read(pid_pipe[0], &actual_pid, sizeof(actual_pid));


                char full_command[1024] = "";

                // 2. Loop through all parts of the command (e.g., "sleep", "5").
                for (int k = 0; k < arr[i].num_cmds; k++) {
                    strcat(full_command, arr[i].cmds[k]); // Add the part
                    if (k < arr[i].num_cmds - 1) {
                        strcat(full_command, " "); // Add a space in between
                    }
                }
                // background_jobs[no_jobs_background] = pid;
                // background_name[no_jobs_background] = strdup(full_command); // Saves "sleep 5"


                // Register background job with correct pgid as the first child's pid
                int printed_job_index = no_jobs_background + 1; // capture before add increments
                job_head = add(job_head, actual_pid, full_command, 'R', actual_pid);

                printf("[%d] %d\n", printed_job_index, actual_pid);

                ///// LLM GENERATED CODE ENDS///
                // background_jobs[no_jobs_background] = pid;
                
            } 
        
    }
}
dup2(saved_stdin, STDIN_FILENO);
dup2(saved_stdout, STDOUT_FILENO);
close(saved_stdin);
close(saved_stdout);
}