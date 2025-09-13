#include"../include/shell.h"
#include"../include/parser.h"
#include"../include/jobs.h"


// in this we will break them acc to the ;,&, or null charactercause then we have the no of jobs 

void jobs_differ(struct Parser P,job* arr,int* job_count){

    // job arr[1024]; 
    // int flag=1;
    // int curr=0;
    int i_index=0;

    //###### LLM GENERATED CODE STARTS######//

    arr[*(job_count)].num_cmds = 0;
    arr[*(job_count)].delimiter = '\0'; 

    // used gpt to find mistakes in my code then it debuged this
    /// #######LLM GENEARATED CODE ENDS #######///
    

    int flag=0;
    for(int i=0;i<P.tokc;i++){
        if(strcmp(P.tokens[i],"&")==0||strcmp(P.tokens[i],";")==0){

            flag=1;
            arr[(*job_count)].cmds[i_index]=NULL;
            arr[(*job_count)++].delimiter=P.tokens[i][0];
            // for next job
            i_index=0; // for next job
            arr[*(job_count)].num_cmds=0;
            continue;
        }
        flag=0;
        arr[*job_count].cmds[i_index++]=strdup(P.tokens[i]);
        arr[*(job_count)].num_cmds++;
        

    }

    if(flag==0){
        // means like for the last command the delimiter is \0
        arr[*(job_count)].cmds[i_index]=NULL;
        arr[(*job_count)++].delimiter='\0';

    }


}