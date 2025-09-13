#include"../include/bgfg.h"
#include"../include/activites.h"
#include"../include/linkedlist.h"
#include"../include/shell.h"
/// ####LLM GENERATED CODE STARTS///// #####
void fg(char** args) {
    background_job* job;
    if (args[1] == NULL) {
        job = find_recent(job_head);
    } else {
        job = find_no(job_head,atoi(args[1]));
    }

    if (!job) {
        printf("fg: No such job\n");
        return;
    }

    printf("%s\n", job->command); // Requirement: Print command name

    // 1. Give terminal control to the job's process group
    tcsetpgrp(STDIN_FILENO, job->pgid);

    // 2. If stopped, send SIGCONT to resume it
    if (job->state == 'S') {
        if (killpg(job->pgid, SIGCONT) < 0) {
            perror("fg: kill (SIGCONT)");
        }
        job->state ='R';
    }

    // 3. Wait for the job to complete or stop again
    current_foreground = job->pgid; // Mark as current foreground job
    int status;
    waitpid(-job->pgid, &status, WUNTRACED);
    current_foreground = -1; // Unmark

    // 4. Take back terminal control
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    // 5. Update job status after waiting
    if (WIFSTOPPED(status)) {
        job->state = 'S';
    } else {
        // Remove completed job from the list
        job_head = remove_job(job->pid, job_head);
    }
}

/**
 * @brief E.4: Resumes a stopped job in the background.
 * Syntax: bg [job_number]
 */
void bg(char** args) {
    background_job* job;
    // JOB NUMBER WILL BE ALWAYS PROVIDED
    job = find_no(job_head,atoi(args[1])); 

    if (!job) {
        printf("bg: No such job\n");
        return;
    }

    if (job->state =='R') {
        printf("bg: Job already running\n");
        return;
    }

    if (job->state == 'S') {
        // Send SIGCONT to resume in background
        if (killpg(job->pgid, SIGCONT) < 0) {
            perror("bg: kill (SIGCONT)");
            return;
        }
        job->state ='R';
        printf("[%d] %s &\n", job->job_id, job->command);
    }
}


/// ####LLM GENERATED CODE ENDS########?///