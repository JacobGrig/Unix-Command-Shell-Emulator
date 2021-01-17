#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "list.h"

#define DEBUG(a) fprintf (stderr, a)

typedef struct cmd_inf {
    char **argv;
    char *infile;
    char *outfile;
    int backgrd;
    int subshell;
    int end_file;
} cmd_inf;

typedef struct commands {
    int opcode;
    int priora;
} commands;

typedef struct tree {
    int opcode;
    char *subshell;
    struct tree *left;
    struct tree *right;
    cmd_inf *execute;
} tree;



enum {
    OP_LEFAR,  /*<*/
    OP_RGTAR,  /*>*/
    OP_DRTAR,  /*>>*/
    OP_PIPE,   /*|*/
    OP_DAMPSD, /*&&*/
    OP_DPIPE,  /*||*/
    OP_AMPSD,  /*&*/
    OP_SEMCOL  /*;*/
};

char *get_line(void);
tree *tree_builder(const list *, const list *, int);
list *redirected_list (list *);
list *back_list (list *);
void tree_free (tree **);
void print_tree (tree *);
