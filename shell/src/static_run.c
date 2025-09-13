#include"../include/shell.h"
#include"../include/lets_go.h"
#include"../include/hop.h"
#include"../include/reveal.h"
#include"../include/log.h"
#include"../include/activites.h"
#include"../include/bgfg.h"


int is_builtin_command(const char* cmd) ;
// backgrp


void execute_pipeline(cmd_grp arr_cmds[], int no_cmds, int is_background,int pid_pipe_fd) {
    

    

    int input_fd = STDIN_FILENO; // The input for the first command is standard input.
    int pids[no_cmds];
    int pid_count = 0;

    // If it's a background process, it shouldn't hold onto the terminal's input.
    if (is_background) {
        int dev_null = open("/dev/null", O_RDONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDIN_FILENO);
            close(dev_null);
        }
        // no_jobs_background++; already did this there nigga 
    }

    // Loop through each command in the job
    for (int j = 0; j < no_cmds; j++) {
        int pipe_fd[2];

        // Create a pipe if this command's output needs to go to the next one.
        if (arr_cmds[j].is_pipe) {
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        // --- OPTIMIZATION: Check for built-in commands BEFORE forking ---
        if (is_builtin_command(arr_cmds[j].arguments[0])) {
            // Save the original stdin and stdout of the shell process
            int original_stdin = dup(STDIN_FILENO);
            int original_stdout = dup(STDOUT_FILENO);

            // 1. Redirect input for the built-in command
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            if (arr_cmds[j].input_fd != STDIN_FILENO) {
                dup2(arr_cmds[j].input_fd, STDIN_FILENO);
                close(arr_cmds[j].input_fd);
            }

            // 2. Redirect output for the built-in command
            if (arr_cmds[j].is_pipe) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
            }
            if (arr_cmds[j].output_fd != STDOUT_FILENO) {
                dup2(arr_cmds[j].output_fd, STDOUT_FILENO);
                close(arr_cmds[j].output_fd);
            }
            
            // 3. Execute the built-in command directly
            if (strcmp(arr_cmds[j].arguments[0], "hop") == 0) {
                hop(arr_cmds[j].arguments, arr_cmds[j].no_arguments);
            } else if (strcmp(arr_cmds[j].arguments[0], "bg") == 0) {
                bg(arr_cmds[j].arguments);
            } else if (strcmp(arr_cmds[j].arguments[0], "log") == 0) {
                logs(arr_cmds[j].arguments, arr_cmds[j].no_arguments);
            }  else if(strcmp(arr_cmds[j].arguments[0], "fg") == 0){
                fg(arr_cmds[j].arguments);
            }
            // 4. Restore the original stdin and stdout for the shell
            dup2(original_stdin, STDIN_FILENO);
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdin);
            close(original_stdout);

            // The next command's input is the read end of the new pipe.
            if (arr_cmds[j].is_pipe) {
                close(pipe_fd[1]);

                input_fd = pipe_fd[0];
                // close(pipe_fd[1]); // The write end is not needed anymore
            }
            continue; // Go to the next command in the loop
        }


        pids[pid_count] = fork();
        if (pids[pid_count] < 0) {
            perror("fork");
            exit(1);
        }

        if (j == 0) {
            // Set process group for the pipeline to the first child's pid
            if (pids[pid_count] > 0) {
                setpgid(pids[pid_count], pids[pid_count]);
                if (is_background) {
                    write(pid_pipe_fd, &pids[pid_count], sizeof(pids[pid_count]));
                }
            }
        } else if (pids[pid_count] > 0) {
            // Subsequent children join the same process group
            setpgid(pids[pid_count], pids[0]);
        }

        if (pids[pid_count] == 0) {
            // --- This is the CHILD process for a single command in the pipeline ---

            // Place child into the pipeline's process group
            if (j == 0) {
                setpgid(0, 0);
            } else {
                setpgid(0, pids[0]);
            }

            // 1. Set up INPUT redirection
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            // Handle file input redirection (e.g., cmd < file.txt)
            if (arr_cmds[j].input_fd != STDIN_FILENO) {
                dup2(arr_cmds[j].input_fd, STDIN_FILENO);
                close(arr_cmds[j].input_fd);
            }

            // 2. Set up OUTPUT redirection
            if (arr_cmds[j].is_pipe) {
                close(pipe_fd[0]); // This child doesn't read from the new pipe.
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
            }
            // Handle file output redirection (e.g., cmd > file.txt)
            if (arr_cmds[j].output_fd != STDOUT_FILENO) {
                dup2(arr_cmds[j].output_fd, STDOUT_FILENO);
                close(arr_cmds[j].output_fd);
            }

            if(strcmp(arr_cmds[j].arguments[0],"reveal")==0){
                reveal(arr_cmds[j].arguments, arr_cmds[j].no_arguments);
            }


            // 3. Execute the command (Only external commands reach here)
            execvp(arr_cmds[j].arguments[0], arr_cmds[j].arguments);
            if (errno == ENOENT) {
                // ENOENT means "No such file or directory"
                fprintf(stderr, "Command not found!\n");
                }


            //perror(arr_cmds[j].arguments[0]); // This runs only if execvp fails
            exit(1);


        } else {
            // --- This is the PARENT process (the pipeline manager) ---

            // Close the write end of the pipe; parent doesn't use it.
            if (arr_cmds[j].is_pipe) {
                close(pipe_fd[1]);
            }
            
            // Close the previous pipe's read end; it's been passed on.
            if (input_fd != STDIN_FILENO) {
                close(input_fd);
            }

            // The next command's input is the read end of the new pipe.
            if (arr_cmds[j].is_pipe) {
                input_fd = pipe_fd[0];
            }
        }
        pid_count++;
    }

    // --- Wait for all children in the pipeline/////to finish ---
    // We only do this if it's a foreground job.
    if (!is_background) {
        for (int j = 0; j < pid_count; j++) {
            waitpid(pids[j], NULL, 0);
        }
    }
}

int is_builtin_command(const char* cmd) {
    if (strcmp(cmd,"hop") == 0 ||
        strcmp(cmd,"log") == 0 || strcmp(cmd,"bg")==0
        || strcmp(cmd,"fg")==0) {
        return 1;
    }
    return 0;
}



///#####LLM GENRATED. CODE ENDS#####/////