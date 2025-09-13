#ifndef ACTIVITES_H
#define ACTIVITES_H

typedef struct background_temp{
    pid_t pid;
    char name[1024];
    char status[20]; // stopped or running
}background_temp;
void activites();

char getstatus(int pid);

#endif