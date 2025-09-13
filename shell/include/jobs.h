#ifndef JOBS
#define JOBS


//// ####LLM GENERATED CODE STARST#######//////////

typedef struct job {
    char delimiter; // Stores the ';' or '&' that ENDS this job
    char* cmds[1024];
    int num_cmds;
} job;

// just ask job to make my struct bette so that i can do things

////// #####LLM GENERATED CODE ENDS########////


void jobs_differ(struct Parser P,job* arr,int* job_count);

#endif