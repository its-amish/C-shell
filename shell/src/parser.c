#include "../include/parser.h"

// ### No changes to helper functions ###
int is_special(char c) {
    return (c == '|' || c == '&' || c == '>' || c == '<' || c == ';');
}

int is_valid_name(const char *t) {
    if (!t || !*t) return 0;
    for (int i = 0; t[i]; i++) {
        if (is_special(t[i]) || isspace((unsigned char)t[i])) return 0;
    }
    return 1;
}

char *peek(struct Parser *P) {
    return (P->pos < P->tokc) ? P->tokens[P->pos] : NULL;
}

char *next(struct Parser *P) {
    return (P->pos < P->tokc) ? P->tokens[P->pos++] : NULL;
}

// ### The ONLY significant change is in tokenize() ###
void tokenize(struct Parser *P, const char s[]) {
    P->tokc = 0;
    P->pos = 0;
    int i = 0;
    int len = strlen(s);

    while (i < len && P->tokc < MAX_TOKENS) {
        while (i < len && isspace((unsigned char)s[i])) i++;
        if (i >= len) break;

        // --- NEW VALIDATION LOGIC ---
        // Before creating a ';' or '&' token, check the preceding character.
        if (s[i] == ';' || s[i] == '&') {
            // It's only valid if it's the very first character or is preceded by whitespace.
            if (i > 0 && !isspace((unsigned char)s[i-1])) {
                // SYNTAX ERROR: e.g., "a;b" or "ls&".
                // We abort tokenization by returning 0 tokens.
                // First, free any tokens we might have already created.
                for (int j = 0; j < P->tokc; j++) {
                    free(P->tokens[j]);
                }
                P->tokc = 0;
                return; // Abort.
            }
        }
        // --- END OF NEW LOGIC ---

        if (i + 1 < len && s[i] == '>' && s[i+1] == '>') {
            P->tokens[P->tokc++] = strndup(&s[i], 2);
            i += 2;
        } else if (is_special(s[i])) {
            P->tokens[P->tokc++] = strndup(&s[i], 1);
            i++;
        } else {
            int start = i;
            while (i < len && !isspace((unsigned char)s[i]) && !is_special(s[i])) {
                i++;
            }
            P->tokens[P->tokc++] = strndup(&s[start], i - start);
        }
    }
}


// ### No changes to any parsing logic functions ###
int parse_atomic(struct Parser *P) {
    char *t = peek(P);
    if (!t || !is_valid_name(t)) return 0;
    next(P);

    while ((t = peek(P))) {
        if (strcmp(t, "<") == 0 || strcmp(t, ">") == 0 || strcmp(t, ">>") == 0) {
            next(P);
            char *filename = next(P);
            if (!filename || !is_valid_name(filename)) return 0;
        } else if (is_special(t[0])) {
            break;
        } else {
            if (!is_valid_name(t)) return 0;
            next(P);
        }
    }
    return 1;
}

int parse_cmd_group(struct Parser *P) {
    if (!parse_atomic(P)) return 0;
    while (peek(P) && strcmp(peek(P), "|") == 0) {
        next(P);
        if (!parse_atomic(P)) return 0;
    }
    return 1;
}

int parse_shell_cmd(struct Parser *P) {
    if (!parse_cmd_group(P)) return 0;

    while (peek(P)) {
        char* token = peek(P);
        if (strcmp(token, "&") != 0 && strcmp(token, ";") != 0) {
            // This is not a separator, so it must be the start of a new, invalid command.
            // e.g. "ls pwd" should fail.
            return 0;
        }
        next(P);
        
        // Handle trailing '&' for background jobs
        if (strcmp(token, "&") == 0 && !peek(P)) {
            return 1;
        }
        
        // A separator must be followed by another command.
        if (!peek(P)) {
            return 0; // e.g. "ls ;"
        }

        if (!parse_cmd_group(P)) {
            return 0;
        }
    }
    return peek(P) == NULL;
}


// ### No changes to public API functions ###
void free_tokens(struct Parser *P) {
    if (!P) return;
    for (int i = 0; i < P->tokc; i++) {
        free(P->tokens[i]);
        P->tokens[i] = NULL;
    }
    P->tokc = 0;
}

int parse_and_validate(struct Parser *P, const char *input) {
    if (input == NULL || input[0] == '\0') return 0;

    tokenize(P, input);

    if (P->tokc == 0) {
        // This now correctly handles syntax errors from the tokenizer.
        return 0;
    }

    P->pos = 0;
    int is_valid = parse_shell_cmd(P);
    
    // After parsing, we must have consumed all tokens.
    if (is_valid && P->pos == P->tokc) {
        P->pos = 0;
        return 1;
    } else {
        free_tokens(P);
        return 0;
    }
}