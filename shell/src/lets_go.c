#include"../include/lets_go.h"
#include"../include/shell.h"
#include"../include/parser.h"
#include"../include/jobs.h"

// so the things is we will break it after jobs and then i have to fix it

// there are two ways of doing things easy or hard but i always like hard cause thats what the state i am in


void letsdoit(char* cmds[],int no_cmds ,cmd_grp* arr,int *curr){ // this curr will give the no of cmd grp it can have


    /// so we need to break the job into its final token we will delimite them by pipes we will break them by pipes 
    int temp=0; //this is for mantaining what do we say the no of pipes or cmds anything but we have to make norn
    int curr_temp=0;
;

    // before this we also have to intialize this
    arr[temp].no_arguments=0;
    arr[temp].input_fd=STDIN_FILENO;
    arr[temp].output_fd=STDOUT_FILENO;
    arr[temp].is_pipe=0;


    for(int i=0;i<no_cmds;i++){
        
        if(strcmp(cmds[i],"|")==0){
            //means yeah that is


            // then what will u do
            // just check that there is pipe or not
            // flag=1;
            arr[temp].arguments[curr_temp++]=NULL;
            arr[temp].no_arguments=curr_temp;
            arr[temp++].is_pipe=1;
            /// i dont know jack shit
            // for next one 
            // intilazing the next one
            curr_temp=0;
            arr[temp].input_fd=STDIN_FILENO;
            arr[temp].output_fd=STDOUT_FILENO;
            arr[temp].is_pipe=0;
            arr[temp].no_arguments=0;
            continue;


        }
            // means we are under normal input
            if(strcmp(cmds[i],"<")==0){
                // this for input
                i++;
                int fd=open(cmds[i],O_RDONLY);
                if(fd<0){
                    printf("No such file or directory\n");
                    return;
                }
                arr[temp].input_fd=fd;
                // will also work for mulitple input in one 
                continue;
            }
            if(strcmp(cmds[i],">")==0){
                i++;
                int output=open(cmds[i],O_CREAT|O_TRUNC|O_WRONLY,0644);
                if(output<0){
                    printf("Unable to create file for writing\n");
                    return;
                }
                arr[temp].output_fd=output;
                continue;
            }

            if(strcmp(cmds[i],">>")==0){
                i++;
                int output=open(cmds[i],O_WRONLY|O_CREAT|O_APPEND,0644);
                if(output<0){
                    printf("Unable to create file for writing\n");
                    return;
                }
                arr[temp].output_fd=output;
                continue;
            }

            // ohk and if it is normal then we will just do 

        // ohk what is up now what are u upto
        // this is when like for normal argument first argument would be what do we say 
        // and also first argument would be the cmd itself
        arr[temp].arguments[curr_temp++]=strdup(cmds[i]); 
            

        // }

    }

// now the thing is just th


arr[temp].arguments[curr_temp]=NULL;
arr[temp].no_arguments=curr_temp;
arr[temp].is_pipe=0; // there cant be pipe like pipe wont be in the end

    
*curr=temp+1; // its jst better like thats


}
