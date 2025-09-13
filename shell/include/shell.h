#ifndef SHELL
#define SHELL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<termios.h>
#include<errno.h>
#include<ctype.h>


typedef struct background_job{
    pid_t pid;
    pid_t pgid; // dont know but gpt gave this to me
    int job_id;
    char command[1024];
    char state;
    struct background_job* next;
}background_job;




extern char psuedo_home[PATH_MAX];// this our psuedo home directory like this is the home directory for the shell
extern char current_working[PATH_MAX];
extern char last_working[PATH_MAX];// this  \0 will tell us that hop command is run once or not 

extern char*last_commands[15];
extern int no_commands;
extern int original_stdin;
extern int original_stdout;
// extern int background_jobs[10000]; // this is for pid 
// extern char* background_name[10000]; // this is for name of the process
 extern int no_jobs_background;


extern pid_t current_foreground; // this is for ctrl+z
extern pid_t shell_pgid; // just dont know whay
extern int next_job_id;
extern background_job background_list[10000];
extern background_job* job_head;
#endif