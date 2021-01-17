#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "analisys.h"
#include "list.h"
#include "executer.h"

char *get_line (void);

int main(int argc, const char **argv)
{
    if (argc == 1) {
        char *line;
        tree *tree_for, *tree_back;
        list *l, *tmp_for, *tmp_back, *l_for, *l_back;
        int status;

        printf("xsh@My-Little-Shell $ ");
        for (line = get_line(); line != NULL; line = get_line ()) {

            l = back_list (redirected_list (str_to_list (line)));

			l_for = list_rarer(l, 0);
			l_back = list_rarer(l, 1);
/*
			printf("list:\n");
			print_list(l);
*/
			l = list_free(l);

			//list *cpy_l = list_cpy(l);
/*
			printf("list for:\n");
			print_list(l_for);
			printf("list back:\n");
			print_list(l_back);
*/
            tmp_for = l_for;
            if (tmp_for) {
                while (tmp_for -> next) {
                    tmp_for = tmp_for -> next;
                }
            }

            tmp_back = l_back;
            if (tmp_back) {
                while (tmp_back -> next) {
                    tmp_back = tmp_back -> next;
                }
            }

            tree_for = tree_builder (l_for, tmp_for, 0);
            tree_back = tree_builder (l_back, tmp_back, 1);
/*
			printf("tree for:\n");
			print_tree(tree_for);
			printf("tree back:\n");
			print_tree(tree_back);
*/
            if (!fork ()) {
                if (!fork ()) {
                    setpgid (0, 0);
                    status = shell (tree_back);
                    exit (status);
                } else {
                    signal (SIGCHLD, SigHndl);
                    exit (0);
                }
            } else {
                wait (NULL);
                shell (tree_for);
				//print_tree(tree_for);

                tree_free (&tree_for);
                tree_free (&tree_back);

                l_for = list_free (l_for);
				l_back = list_free (l_back);
                free (line);
                printf ("xsh@My-Little-Shell $ ");
            }
        }
        return 0;
    } else if (argc == 2) {

		//printf("argv[1]=%s", argv[1]);
        char *line;
        str_cpy (argv[1], &line);
        if (!line || line[0] == '\0') {
            fprintf (stderr, "xsh: syntax error near ')'\n");
            return 1;
        }
        tree *tree_for, *tree_back;
        list *l, *tmp_for, *tmp_back, *l_for, *l_back;
        int status;

        l = back_list (redirected_list (str_to_list (line)));

		l_for = list_rarer(l, 0);
		l_back = list_rarer(l, 1);

		l = list_free(l);

		tmp_for = l_for;
        if (tmp_for) {
            while (tmp_for -> next) {
                tmp_for = tmp_for -> next;
            }
        }

        tmp_back = l_back;
        if (tmp_back) {
            while (tmp_back -> next) {
                tmp_back = tmp_back -> next;
            }
        }

        tree_for = tree_builder (l_for, tmp_for, 0);
        tree_back = tree_builder (l_back, tmp_back, 1);


        if (!fork ()) {
            if (!fork ()) {
                setpgid (0, 0);
                status = shell (tree_back);
                exit (status);
            } else {
                signal (SIGCHLD, SigHndl);
                exit (0);
            }
        } else {
            wait (NULL);
            shell (tree_for);

            tree_free (&tree_for);
            tree_free (&tree_back);

            l_for = list_free (l_for);
			l_back = list_free (l_back);
            free (line);
        }
    } else {
        fprintf (stderr, "Error in main(): too many arguments!\n");
        return 1;
    }
}

char *get_line(
    void)
{
    char *line = NULL;
    int i = 0, c;
    for (c = getchar (); c != '\n' && c != EOF; c = getchar ()) {
        i++;
        line = realloc (line, sizeof(char) * i);
        line[i - 1] = c;
    }
    line = realloc (line, sizeof(char) * (i + 1));
    line[i] = '\0';
    if (c == EOF) {
        free (line);
        return NULL;
    }
    return line;
}
