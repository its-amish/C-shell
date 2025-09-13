

#include"../include/shell.h"
#include"../include/parser.h"
// this is for storing logs from  previous sessions

static void get_log_file_path(char* buffer, size_t size) {
    snprintf(buffer, size, "%s/logs.txt", psuedo_home);
}

void store_logs() {
    char log_file_path[PATH_MAX];
    get_log_file_path(log_file_path, sizeof(log_file_path));
    
    FILE *log_file = fopen(log_file_path, "r");

    // If the file doesn't exist or can't be opened, fopen returns NULL.
    // This is the normal case for the first run, so we just return.
    if (log_file == NULL) {
        return;
    }

    char buffer[1025];
    no_commands = 0;

    // Read lines directly from the log_file stream until we reach the end of the file
    // or fill our command history.
    while (no_commands < 15 && fgets(buffer, sizeof(buffer), log_file) != NULL) {
        // fgets reads the newline character, so we should remove it.
        buffer[strcspn(buffer, "\n")] = 0;

        // Skip empty lines
        if (strlen(buffer) == 0) {
            continue;
        }

        // strdup allocates memory and copies the string.
        last_commands[no_commands] = strdup(buffer);
        no_commands++;
    }

    // Always close the file when you're done with it.
    fclose(log_file);
}

void write_back();
void store_new(char* command, struct Parser P){
    // Check if command is empty or just whitespace
    if (command == NULL || strlen(command) == 0) {
        return;
    }
    
    // Skip if command contains "log" as any token
    for(int i = 0; i < P.tokc; i++){
        if(strcmp(P.tokens[i], "log") == 0){
            return;
        }
    }
    
    // If this is the first command, just add it
    if(no_commands == 0){
        last_commands[no_commands++] = strdup(command);
        write_back();
        return;
    }
    
    // Check if command is identical to the previous one
    if(strcmp(command, last_commands[no_commands-1]) == 0){
        return; // Don't store identical commands
    }
    
    // If we're at capacity (15 commands), shift everything left and add new one
    if(no_commands == 15){
        free(last_commands[0]);
        for(int i = 1; i < 15; i++){
            last_commands[i-1] = last_commands[i];
        }
        last_commands[14] = strdup(command);
    } else {
        // Just add the new command
        last_commands[no_commands++] = strdup(command);
    }
    
    write_back();
}   

void write_back(){
    char log_file_path[PATH_MAX];
    get_log_file_path(log_file_path, sizeof(log_file_path));

    // Use O_TRUNC to clear the file before writing the new history.
    int fd = open(log_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error opening log file for writing");
        return;
    }

    // Temporarily redirect stdout to the file.
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);

    for (int i = 0; i < no_commands; i++) {
        if (last_commands[i] != NULL) {
            printf("%s\n", last_commands[i]);
        }
    }
    fflush(stdout);

    // Restore original stdout and close the file.
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    close(fd);
}