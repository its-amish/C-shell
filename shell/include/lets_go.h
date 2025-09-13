#ifndef LETSGO
#define LETSGO

// ###LLM genrated code starts?#######////

typedef struct cmd_grp{
    int no_arguments;
    // char cmd[1024]; // no need for this since the first argument would be cmd always
    char* arguments[1024];
    int output_fd;
    int input_fd;
    int is_pipe; // does it contains pipe or not
}cmd_grp;


// like i just have asked what can i make changes to make my struct more good and robust

////###LLM Generated code end ########//



void letsdoit(char* cmds[],int no_cmds ,cmd_grp* arr,int *curr);

#endif