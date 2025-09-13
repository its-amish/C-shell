// #####LLM GENRATED CODE######///

#ifndef PARSER
#define PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 1024
// Parser state kept in a struct so it's not global
struct Parser {
    char *tokens[MAX_TOKENS];
    int tokc;
    int pos;
};
int is_special(char c);
int is_valid_name(const char *t);
void tokenize(struct Parser *P, const char s[]);
char *peek(struct Parser *P);
char *next(struct Parser *P);
void free_tokens(struct Parser *P);
int parse_atomic(struct Parser *P);
int parse_cmd_group(struct Parser *P);
int parse_shell_cmd(struct Parser *P);
int check_valid(const char* input);
int parse_and_validate(struct Parser *P, const char *input);


#endif


// #####LLM GENERATED CODE#######// 