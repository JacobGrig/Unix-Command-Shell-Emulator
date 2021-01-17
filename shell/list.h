#pragma once

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct list {
    int opcode;
    char *token;
    char *infile;
    char *outfile;
    int end_file;
    int subshell;
    int backgrd;
    struct list *next;
    struct list *prev;
} list;

int str_len (const char *);
void str_cpy (const char *, char **);
list *ins_elem (list *, const char *, int, int);
list *ins_token (list *, const char *);
list *ins_token_with_sub (list *, const char *);
list *ins_opcode (list *, int);
list *list_free (list *);
void del_elem (list **, list **);
list* list_cpy (list *);
list* list_rarer (list *l, int backgrd);
list *str_to_list (const char *);
void print_list (const list *);
void print_rev_list (const list *);
