#ifndef STORE_LOGS
#define STORE_LOGS

void store_logs(); // this is for storing it from previous session
void store_new(char* command,struct Parser P);
void write_back();
#endif