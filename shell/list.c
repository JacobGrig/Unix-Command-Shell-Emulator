#include <stdlib.h>

#include "list.h"
#include "analisys.h"

int str_len (const char *src) {
    if (!src) {
        return -1;
    }
    int i = 0;
    while (src[i++]) {
        ;
    }
    return i - 1;
}

void str_cpy (const char *src, char **dst)
{
    int i = 0;

    if (src == NULL) {
        *dst = NULL;
        return;
    }
    *dst = malloc (str_len (src) + 1);
    while (1) {
        (*dst)[i] = src[i];
        if (src[i++] == '\0') {
            break;
        }
    }
}

list *ins_elem (
    list *l,
    const char *token,
    int opcode,
    int subshell)
{
    list *tmp = malloc (sizeof (list));

	int changed = 0;

	if (token) {
		if (!strcmp(token, "$HOME")) {
			str_cpy (getenv("HOME"), &(tmp -> token));
			changed = 1;
		}
		if (!strcmp(token, "$SHELL")) {
			str_cpy (getenv("PWD"), &(tmp -> token));
			changed = 1;
		}
		if (!strcmp(token, "$USER")) {
			str_cpy (getlogin(), &(tmp -> token));
			changed = 1;
		}
		if (!strcmp(token, "$EUID")) {
			int euid = geteuid(), euid_help = euid;
			int length = 1;
			while (euid_help) {
				euid_help /= 10;
				++length;
			}
			if (length == 1) {
				++length;
			}
		
			char *new_token = malloc(sizeof(char) * length);
			sprintf(new_token, "%d", euid);
			new_token[length - 1] = '\0';
			str_cpy (new_token, &(tmp -> token));
			free(new_token);
			changed = 1;
		}
	}
    if (!changed) {
		str_cpy (token, &(tmp -> token));
	}
    tmp -> opcode = opcode;
    tmp -> infile = NULL;
    tmp -> outfile = NULL;
    tmp -> next = l;
    tmp -> prev = NULL;
    tmp -> end_file = 0;
    tmp -> subshell = subshell;
    tmp -> backgrd = 0;
    if (l) {
        l -> prev = tmp;
    }
    return tmp;
}

list *ins_token (
    list *l,
    const char *token)
{
    return ins_elem (l, token, -1, 0);
}

list *ins_token_with_sub (
    list *l,
    const char *token)
{
    return ins_elem (l, token, -1, 1);
}

list *ins_opcode (
    list *l,
    int opcode)
{
    return ins_elem (l, NULL, opcode, 0);
}

list *list_free (
    list *l)
{
    if (!l) {
        return NULL;
    }

    list *tmp = l;
    l = l -> next;
    if (tmp -> token) {
        free (tmp -> token);
    }
    if (tmp -> infile) {
        free (tmp -> infile);
    }
    if (tmp -> outfile) {
        free (tmp -> outfile);
    }
    free (tmp);
    return list_free (l);
}

void del_elem (
    list **l,
    list **elem_ptr)
{
    list *tmp1, *tmp2;
    if (!(*elem_ptr)) {
        DEBUG ("Error in del_elem_without_free(): null list");
    }
    if ((*elem_ptr) -> next) {
        if ((*elem_ptr) -> prev) {
            tmp1 = (*elem_ptr) -> prev;
            tmp2 = (*elem_ptr) -> next;
            tmp1 -> next = tmp2;
            tmp2 -> prev = tmp1;
        } else {
            tmp1 = (*elem_ptr) -> next;
            (*l) = tmp1;
            tmp1 -> prev = NULL;
        }
    } else if ((*elem_ptr) -> prev) {
        tmp2 = (*elem_ptr) -> prev;
        tmp2 -> next = NULL;
    } else {
		(*l) = NULL;
	}
    if ((*elem_ptr) -> token) {
        free ((*elem_ptr) -> token);
    }
    if ((*elem_ptr) -> infile) {
        free ((*elem_ptr) -> infile);
    }
    if ((*elem_ptr) -> outfile) {
        free ((*elem_ptr) -> outfile);
    }

    free (*elem_ptr);
}

list* list_cpy (
	list *src)
{
	list *dst = NULL;
	list *tmp1 = malloc(sizeof(list));
	list *tmp2 = src;
	list *prev = NULL;
	while (tmp2) {
		tmp1->opcode = tmp2->opcode;
		str_cpy(tmp2->token, &(tmp1->token));
		str_cpy(tmp2->infile, &(tmp1->infile));
		str_cpy(tmp2->outfile, &(tmp1->outfile));
		tmp1->end_file = tmp2->end_file;
		tmp1->subshell = tmp2->subshell;
		tmp1->backgrd = tmp2->backgrd;
		if (!dst) {
			dst = tmp1;
			dst->prev = NULL;
		}
		prev = tmp1;
		tmp1->next = malloc(sizeof(list));
		tmp1 = tmp1->next;
		tmp1->prev = prev;
		tmp2 = tmp2->next;
	}
	free(tmp1);
	if (prev) {
		prev->next = NULL;
	}
	return dst;
}

list* list_rarer (
	list *l, 
	int backgrd)
{
	list *res_list = list_cpy(l);
	list *tmp = res_list;
	list *next = NULL;

	while (tmp) {
		next = tmp->next;
		if (tmp->backgrd != backgrd) {
			del_elem(&res_list, &tmp);
		}
		tmp = next;
	}
	
	return res_list;
}

list *str_to_list (
    const char *string)
{
    if (!string) {
        return NULL;
    }

    list *l = NULL;
    int i = 0, j = 0, tmp, oper, open_brc = 0;
	int open_quo = 0;
	int backslash = 0;
    char *token = NULL;

    while ((tmp = string[i++])) {
        switch (tmp) {
			case '\"':
				if (!backslash) {
					open_quo = !open_quo;
				} else {
					token = realloc (token, j + 1);
                    token[j++] = tmp;
					backslash = 0;
				}
				break;
			case '\\':
				if (backslash) {
					token = realloc (token, j + 1);
                    token[j++] = tmp;
					backslash = 0;
                    break;
				}
				backslash = 1;
				break;
            case '(':
				if (!backslash && !open_quo) {
		            open_brc++;
		            if (j > 0) {
		                token = realloc (token, j + 1);
		                token[j] = '\0';
		                l = ins_token (l, token);
		                free (token);
		                token = NULL;
		                j = 0;
		            }
					int prev = '(';
		            while (open_brc > 0) {
		                token = realloc (token, j + 1);
		                token[j++] = tmp;
		                tmp = string[i++];
		                if (!tmp) {
		                    break;
		                }
		                if (tmp == ')' && prev != '\\') {
		                    open_brc--;
		                }
		                if (tmp == '(' && prev != '\\') {
		                    open_brc++;
		                }
						prev = tmp;
		            }
		            if (open_brc > 0) {
		                DEBUG ("xsh: syntax error near '('\n");
		                l = list_free (l);
		                return NULL;
		            } else {
		                token = realloc (token, j + 2);
		                token[j] = ')';
		                token[j + 1] = '\0';
		                l = ins_token_with_sub (l, token);
		                free (token);
		                token = NULL;
		                j = 0;
		            }
				} else {
					token = realloc (token, j + 1);
                    token[j++] = tmp;
					backslash = 0;
				}
                break;
            case ')':
				if (!backslash && !open_quo) {
		            DEBUG ("xsh: syntax error near ')'\n");
		            l = list_free (l);
		            return NULL;
				} else {
					token = realloc (token, j + 1);
                    token[j++] = tmp;
					backslash = 0;
				}
                break;
            case ';':
            case '|':
            case '&':
            case '<':
            case '>':
			case '#':
			case ' ':
				if (backslash || open_quo) {
					token = realloc (token, j + 1);
                    token[j++] = tmp;
					backslash = 0;
					break;
				}
	            if (j > 0) {
	                token = realloc (token, j + 1);
	                token[j] = '\0';
	                l = ins_token (l, token);
	                free (token);
	                token = NULL;
	                j = 0;
	            }
                if (tmp == ' ') {
                    break;
                }
				if (tmp == '#') {
					return l;
				}
                if (tmp == '&' || tmp == '|' || tmp == '>') {
                    if (string[i] == tmp) {
                        i++;
                        oper = (tmp == '|')? OP_DPIPE  :
                               (tmp == '&')? OP_DAMPSD :
                                             OP_DRTAR;
                    } else {
                        oper = (tmp == '|')? OP_PIPE   :
                               (tmp == '&')? OP_AMPSD  :
                                             OP_RGTAR;
                    }
                } else {
                    oper = (tmp == ';')? OP_SEMCOL :
                                         OP_LEFAR;

                }
                l = ins_opcode (l, oper);
                break;
            default:
				backslash = 0;
                token = realloc (token, j + 1);
                token[j++] = tmp;
                break;
        }
    }
	if (open_quo) {
		DEBUG ("xsh: syntax error near '\"'\n");
    	l = list_free (l);
    	return NULL;
	}
    if (j > 0) {
        token = realloc (token, sizeof (char) * (j + 1));
        token[j] = '\0';
        l = ins_token (l, token);
        free (token);
    }
    return l;
}

void print_list (
    const list *l)

{
    if (l) {
        printf ("token = %s, opcode = %d, backgrd = %d\n", l -> token, l -> opcode, l -> backgrd);
        print_list (l -> next);
    }
}

void print_rev_list (
    const list *l)
{
    if (l) {
        while (l -> next) {
            l = l -> next;
        }
        while (l) {
            printf("token = %s, opcode = %d, infile = %s, outfile = %s, subshell = %d, end_file = %d, backgrd = %d\n", l -> token, l -> opcode, l -> infile, l -> outfile, l -> subshell, l -> end_file, l -> backgrd);
            l = l -> prev;
        }
    }
}
