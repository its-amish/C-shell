


// #include"shell.h"
#include"../include/input.h"
// #include"execute.h"
#include"../include/parser.h"
#include"../include/store_logs.h"
// #include"activites.h"
// #include"linkedlist.h" 
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


char psuedo_home[PATH_MAX];
char current_working[PATH_MAX];
char last_working[PATH_MAX] = { '\0' };

char* last_commands[15];
int no_commands = 0;
int original_stdout;
int original_stdin;
int background_jobs[10000];
char* background_name[10000];
int no_jobs_background=0;

pid_t current_foreground; // this is for ctrl+z
pid_t shell_pgid; // just dont know whay
int next_job_id;
background_job* job_head;


void handle_ctrld() {
    printf("logout\n");
    fflush(stdout);
    // background_job* current = job_head;
    // background_job* next = NULL;
    background_job* temp=job_head;
    while(temp!=NULL){
        kill(temp->pgid,SIGKILL);
        // job_head=remove_job(temp->pid,job_head);
        temp=temp->next;
    }

    exit(0);
}



void handle_ctrlc(int sig) {
    if (current_foreground > 0) {
        killpg(current_foreground, SIGINT);
    }
    printf("\n");// foir next line
}



void handle_ctrlz(int sig) {
    if (current_foreground > 0) {
        killpg(current_foreground, SIGTSTP);
    }
    // job_head=add(job_head,)

    // printf("\n");
}


void background_job_removerandchecker(){
    background_job* temp = job_head;
    background_job* prev = NULL;
    
    while(temp != NULL){
        int status;
        pid_t result = waitpid(temp->pid, &status, WNOHANG);
        
        if(result == temp->pid){
            // Process has changed state // so the thing is 
            if(WIFEXITED(status) || WIFSIGNALED(status)){
                // Process has terminated
                if(WIFEXITED(status)){
                    int exit_code = WEXITSTATUS(status);
                    if (exit_code == 0) {
                        printf("%s with pid %d exited normally\n", temp->command, temp->pid);
                    } else {
                        printf("%s with pid %d exited abnormally\n", temp->command, temp->pid);
                    }
                } else {
                    printf("%s with pid %d terminated by signal %d\n", 
                           temp->command, temp->pid, WTERMSIG(status));
                }
                
                // Remove from list
                if(prev == NULL){
                    job_head = temp->next;
                } else {
                    prev->next = temp->next;
                }
                background_job* to_free = temp;
                temp = temp->next;
                free(to_free);
                continue; // Don't advance prev
                
            } else if(WIFSTOPPED(status)){
                if(temp->state != 'T'){
                    temp->state = 'T';
                    printf("[%d] Stopped %s\n", temp->job_id + 1, temp->command);
                }
            }
        }
        
        prev = temp;
        temp = temp->next;
    }
}




int main(){
    // we want to run the thing that is called the thin
    //  printf("# I am running my own shell\n");/
    // sleep(5);

    // sleep(1000000);
    original_stdout=dup(STDOUT_FILENO);
    original_stdin=dup(STDIN_FILENO);
    getcwd(psuedo_home,sizeof(psuedo_home)); // this is where we are starting from so it is better if u just have it saved 
    job_head=NULL;

    // Set up job control
    shell_pgid = getpid();

  

    // Set up signal handlers
    signal(SIGINT, handle_ctrlc);   // Ctrl-C handler
    signal(SIGTSTP, handle_ctrlz); // Ctrl-Z handler (using renamed function)
    signal(SIGTTOU, SIG_IGN);      // Ignore terminal output from background processes
    signal(SIGTTIN, SIG_IGN);      // Ignore terminal input from background processes
    signal(SIGQUIT, SIG_IGN);
    
    store_logs(); // this function is for storing the previous session log commands
    while(1){
       
        default_state();
        fflush(stdout);
        // printf("hi i am amish\n");
        char arr_input[1025];
        fgets(arr_input,1025,stdin);

        if(feof(stdin)){
            // handle_sigquit(SIGQUIT);  // Handle Ctrl-D by logging out
            handle_ctrld();
        }


        // printf(" i just taked the input\n");
        arr_input[strcspn(arr_input, "\n")] = 0; // Find and remove the newline // gpt told mw do this shit
        background_job_removerandchecker();

        struct Parser P; // now we have tokeinze it fdo i
        
        int valid=parse_and_validate(&P,arr_input);
        store_new(arr_input,P); // this way we will store new logs
        if(valid==0){
            printf("Inavlid Syntax!\n");
            continue;
        }

        


        execute(P);    
        
        for (int i = 0; i < P.tokc; i++) { // LLM GENERATION LIKE CAUSE IT IS WHAT IT IS
            free(P.tokens[i]);
        }
        //break;
        // break;
    }
    return 0;

}