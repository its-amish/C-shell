#include "../include/shell.h"
#include "../include/log.h"
#include "../include/execute.h"
#include "../include/parser.h"
#include "../include/store_logs.h"

void logs(char* tokens[], int tokc) {
    // Check for invalid syntax first
    // for(int i=0;i<tokc;i++){
    //     printf("%s\n",tokens[i]);
    // }
    if (tokc > 3 || (tokc == 3 && strcmp(tokens[1], "execute") != 0)) {
        printf("log: Invalid Syntax!\n");
        return;
    }
    
    if (tokc == 1) {
        // Default 'log' command: print history in order of oldest to newest
        for (int i = 0; i < no_commands; i++) {
            printf("%s\n", last_commands[i]);
        }
        return;
    }

    // Handle subcommands
    if (strcmp(tokens[1], "purge") == 0) {
        if (tokc != 2) {
            printf("log: Invalid Syntax!\n");
            return;
        }
        // Clear the history
        for (int i = 0; i < no_commands; i++) {
            if (last_commands[i] != NULL) {
                free(last_commands[i]);
                last_commands[i] = NULL;
            }
        }
        no_commands = 0;
        write_back(); // Write the now-empty history back to the file

    } else if (strcmp(tokens[1], "execute") == 0) {
        if (tokc != 3) {
            printf("log: Invalid Syntax!\n");
            return;
        }

        int index = atoi(tokens[2]);
        if (index <= 0 || index > no_commands) {
            printf("log: Invalid Syntax!\n");
            return;
        }

        // Convert from newest-to-oldest index to array index
        // Index 1 = newest = last_commands[no_commands - 1]
        // Index 2 = second newest = last_commands[no_commands - 2]
        // etc.
        char* command_to_run = last_commands[no_commands - index];
        // printf("Executing: %s\n", command_to_run);

        // Parse and execute the historical command
        struct Parser p;
        int valid = parse_and_validate(&p, command_to_run);
        
        if (valid) {
            execute(p);
            // Clean up parser memory
            for (int i = 0; i < p.tokc; i++) {
                free(p.tokens[i]);
            }
        } else {
            printf("Invalid Syntax in stored command!\n");
        }
        
    } else {
        printf("log: Invalid Syntax!\n");
    }
}