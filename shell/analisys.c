#include "analisys.h"
#include "list.h"

static int get_priora (
    int opcode)
{
    if (opcode == -1) {
        return -1;
    }
    const commands list_cmds[] = {
        {OP_LEFAR,  3},
        {OP_RGTAR,  3},
        {OP_DRTAR,  3},
        {OP_PIPE,   2},
        {OP_DAMPSD, 1},
        {OP_DPIPE,  1},
        {OP_AMPSD,  0},
        {OP_SEMCOL, 0},
        {-1,       -1}
    };
    int i = 0;
    while (list_cmds[i].opcode != -1) {
        if (list_cmds[i].opcode == opcode) {
            return list_cmds[i].priora;
        }
        i++;
    }
    return -1;
}

static int sub_end_equal (
    const list *strtl,
    const list *endl)
{
    int flag = 1;

    if (!strtl || !endl) {
        DEBUG ("Error in parse_equal: null strings\n");
    }
    while (strtl != endl) {
        if (strtl -> subshell != endl -> subshell ||
            strtl -> end_file != endl -> end_file) {
            flag = 0;
            break;
        }
        strtl = strtl -> next;
    }
    return flag;
}

static cmd_inf *get_cmd_from_tokens (
    const list *strtl,
    const list *endl,
    int backgrd)
{
    if (!strtl || !endl) {
        DEBUG ("Error in get_cmd_from_token(): null tokens\n");
        return NULL;
    }

    if (!sub_end_equal (strtl, endl)) {
        fprintf (stderr, "xsh: syntax error near %s\n", endl -> token);
        return NULL;
    }

    cmd_inf *cmd = malloc (sizeof (cmd_inf));
    char **argv  = NULL;
    int i = 0;

    str_cpy (endl -> infile, &(cmd -> infile));
    str_cpy (endl -> outfile, &(cmd -> outfile));
    cmd -> backgrd = backgrd;
    cmd -> subshell = endl -> subshell;
    cmd -> end_file = endl -> end_file;

    while (1) {
        if (endl -> backgrd == backgrd) {
            argv = realloc (argv, sizeof (char *) * (i + 1));
            str_cpy (endl -> token, &argv[i++]);
        }
        if (strtl == endl) {
            break;
        }
        endl = endl -> prev;
    }
    argv = realloc (argv, sizeof (char *) * (i + 1));
    argv[i] = NULL;
    cmd -> argv = argv;
    return cmd;
}

list *redirected_list (
    list *l)
{
    if (!l) {
        return NULL;
    }

    list *tmp_list = l, *min_ptr, *ptr_file;
    int prior = 3, dir_in;

    do {
        dir_in = 0;
        while (tmp_list) {
            int local_op = tmp_list -> opcode;
            switch (local_op) {
                case OP_LEFAR:
                case OP_RGTAR:
                case OP_DRTAR:
                    min_ptr = tmp_list;
                    dir_in = 1;
                    break;
            }
            if (dir_in) {
                break;
            }
            tmp_list = tmp_list -> next;
        }
        if (dir_in) {
            ptr_file = min_ptr -> prev;
            if (!ptr_file || ptr_file -> opcode != -1) {
                DEBUG ("Error in redirected_list(): file should be after redirection\n");
                return l;
            }

            char *infile = NULL, *outfile = NULL;
            int end_file = 0;

            switch (min_ptr -> opcode) {
                case OP_LEFAR:
                    infile = ptr_file -> token;
                    break;
                case OP_RGTAR:
                case OP_DRTAR:
                    outfile = ptr_file -> token;
                    if (min_ptr -> opcode == OP_DRTAR) {
                        end_file = 1;
                    }
                    break;
                default:
                    DEBUG ("Error in redirected_list(): unknown operation\n");
                    break;
            }
            list *left_list = min_ptr -> next;
            int prior_left = (left_list)?
                get_priora (left_list -> opcode) : -1;
            while (left_list) {
                if (prior_left < prior && prior_left != -1) {
                    break;
                } else {
                    if (infile) {
                        if (left_list -> infile) {
                            free (left_list -> infile);
                        }
                        str_cpy (infile, &(left_list -> infile));
                    }
                    if (outfile) {
                        if (left_list -> outfile) {
                            free (left_list -> outfile);
                        }
                        str_cpy (outfile, &(left_list -> outfile));
                    }
                    left_list -> end_file = end_file;
                }
                left_list = left_list -> next;
                prior_left = (left_list)?
                    get_priora (left_list -> opcode) : -1;
            }
            list *right_list = min_ptr -> prev;
            int prior_right = (right_list)?
                get_priora (right_list -> opcode) : -1;
            while (right_list) {
                if (prior_right < prior && prior_right != -1) {
                    break;
                } else {
                    if (infile) {
                        if (right_list -> infile) {
                            free (right_list -> infile);
                        }
                        str_cpy (infile, &(right_list -> infile));
                    }
                    if (outfile) {
                        if (right_list -> outfile) {
                            free (right_list -> outfile);
                        }
                        str_cpy (outfile, &(right_list -> outfile));
                    }
                    right_list -> end_file = end_file;
                }
                right_list = right_list -> prev;
                prior_right = (right_list)?
                    get_priora (right_list -> opcode) : -1;
            }
            del_elem (&l, &min_ptr);
            del_elem (&l, &ptr_file);
        }
        tmp_list = l;
    } while (dir_in);
    return l;
}

list *back_list (
    list *l)
{
    if (!l) {
        return NULL;
    }

    list *tmp = l;
    while (tmp) {
        if (tmp -> opcode == OP_AMPSD) {
            tmp -> backgrd = 1;
            tmp = tmp -> next;
            while (tmp &&
                   (get_priora (tmp -> opcode) > get_priora (OP_AMPSD) ||
                    get_priora (tmp -> opcode) == -1)) {
                tmp -> backgrd = 1;
                tmp = tmp -> next;
            }
        }
        if (!tmp) {
            break;
        }
        if (tmp -> opcode != OP_AMPSD) {
            tmp = tmp -> next;
        }
    }
    return l;
}

static int mode_in (
    const list *strtl,
    const list *endl,
    int backgrd)
{
    if (!strtl || !endl) {
        return 0;
    }
    while (1) {
        if (strtl -> backgrd == backgrd) {
            return 1;
        }
        if (strtl == endl) {
            break;
        }
        strtl = strtl -> next;
        if (!strtl) {
            DEBUG ("Error in mode_in(): bad lists\n");
        }
    }
    return 0;
}

tree *tree_builder (
    const list *strtl,
    const list *endl,
    int backgrd)
{
    if (!strtl || !endl) {
        return NULL;
    }
    int min_prior = -1;
    if (!mode_in (strtl, endl, backgrd)) {
        return NULL;
    }
    list *min_ptr = NULL, *local_ptr = (list *) strtl;
    tree *res_tree = malloc (sizeof (tree));
    res_tree -> execute = NULL;
    res_tree -> left = NULL;
    res_tree -> right = NULL;

    while (1) {
        int local_op = local_ptr -> opcode;
        int local_back = local_ptr -> backgrd;
        int local_prior = get_priora (local_op);

        if (local_back == backgrd &&
            local_prior != -1     &&
            (min_prior == -1 || local_prior < min_prior)) {
                min_prior = local_prior;
                min_ptr = local_ptr;
        }

        if (local_ptr == endl) {
            break;
        }

        if (!local_ptr -> next) {
            tree_free (&res_tree);
            return NULL;
        }

        local_ptr = local_ptr -> next;
    }
    if (min_prior != -1) {
        res_tree -> opcode = min_ptr -> opcode;
        res_tree -> execute = NULL;
        res_tree -> left = (min_ptr -> next)?
            tree_builder (min_ptr -> next, endl, backgrd) :
            NULL;
        res_tree -> right = (min_ptr -> prev)?
            tree_builder (strtl, min_ptr -> prev, backgrd) :
            NULL;
    } else {
		//printf("I'm here!\n%s\n%s\n", strtl->token, endl->token);
        res_tree -> opcode = -1;
        res_tree -> execute = get_cmd_from_tokens (strtl, endl, backgrd);
        res_tree -> left = NULL;
        res_tree -> right = NULL;
    }
    return res_tree;
}

static void free_argv (
    char ***argv)
{
    if (!(*argv)) {
        return;
    }
    int i = 0;

    while ((*argv)[i]) {
        free ((*argv)[i++]);
    }
    free (*argv);
}

static void cmd_free (
    cmd_inf **ci)
{
    if (!(*ci)) {
        return;
    }
    if ((*ci) -> infile) {
        free ((*ci) -> infile);
    }
    if ((*ci) -> outfile) {
        free ((*ci) -> outfile);
    }
    if ((*ci) -> argv) {
        free_argv (&((*ci) -> argv));
    }
    free (*ci);
}

void tree_free (
    tree **t)
{
    if (!(*t)) {
        return;
    }

    tree *left = (*t) -> left, *right = (*t) -> right;
    cmd_free (&((*t) -> execute));
    free (*t);
    (*t) = NULL;
    tree_free (&left);
    tree_free (&right);
}

static void print_argv (
    char **argv)
{
    if (argv) {
        int i = 0;

        while (argv[i]) {
            printf ("%s ", argv [i++]);
            fflush (stdout);
        }
    }
}

static void print_cmd (
    cmd_inf *cmds)
{
    if (cmds) {
        print_argv (cmds -> argv);
        printf ("bck = %d, end = %d, in = %s, out = %s, sub = %d\n",
            cmds -> backgrd, cmds -> end_file, cmds -> infile, cmds -> outfile, cmds -> subshell);
    }
}

void print_tree (
    tree *t)
{
    if (t) {
        print_tree (t -> left);
        printf ("%d\n", t -> opcode);
        print_cmd (t -> execute);
        print_tree (t -> right);
    }
}
