#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "executer.h"
#include "analisys.h"

static int equal_files (
    int fd1,
    int fd2)
{
    struct stat st1, st2;
    if (fstat (fd1, &st1) < 0) {
        return -1000;
    }
    if (fstat (fd2, &st2) < 0) {
        return -100500;
    }
    return st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino;
}

void SigHndl (int SigNum) {
    if (SigNum == SIGCHLD) {
        wait (NULL);
    }
}

int shell (
    tree *tree)
{
    if (!tree) {
        return 0;
    }
    int op = tree -> opcode;
    int status;

    switch (op) {
        case OP_SEMCOL:
        case OP_AMPSD:
            if (!tree -> left) {
                fprintf (stderr, "xsh: syntax error near '%c'\n", (op == OP_AMPSD)? '&' : ';');
                return 1;
            }
            if (!tree -> right) {
                return shell (tree -> left);
            }
            shell (tree -> left);
            return shell (tree -> right);

        case OP_PIPE:
            if (!tree -> left || !tree -> right) {
                fprintf (stderr, "xsh: syntax error near '|'\n");
                return 1;
            }
            int fd[2], in, out;

            pipe (fd);

            in = dup (0);
            out = dup (1);

            if (fork()) {
				close (fd[1]);
                dup2 (fd[0], 0);
                //close (fd[1]);
                close (fd[0]);
                shell (tree -> right);
				dup2(in, 0);
                wait (&status);
                //dup2 (in, 0);
				close(in);
                dup2 (out, 1);
				close(out);
                return (WIFEXITED (status))? WEXITSTATUS (status) : -100500;
            } else {
				close (fd[0]);
                dup2 (fd[1], 1);
                //close (fd[0]);
                close (fd[1]);
                status = shell (tree -> left);
                dup2 (in, 0);
				close(in);
                dup2 (out, 1);
				close(out);
                exit (status);
            }

        case OP_DAMPSD:
            if (!tree -> left || !tree -> right) {
                fprintf (stderr, "syntax error near '&&'\n");
                return 1;
            }

            status = shell (tree -> left);

            if (!status) {
                return shell (tree -> right);
            }
            return status;
        case OP_DPIPE:
            if (!tree -> left || !tree -> right) {
                fprintf (stderr, "syntax error near '||'\n");
                return 1;
            }

            status = shell (tree -> left);

            if (status) {
                return shell (tree -> right);
            }
            return status;
        default:
            return execute_cmd (tree -> execute);
    }
}

int execute_cmd (
    const cmd_inf *cmds)
{
    int status;
    char **argv = cmds -> argv;
    int rdr = -1, wrtr = -1;
    int in, out;

    if (cmds -> infile) {
        if ((rdr = open (cmds -> infile, O_RDONLY, 0777)) < 0) {
            fprintf (stderr, "xsh: error input file '%s'\n", cmds -> infile);
            return 1;
        }
        in = dup (0);
        dup2 (rdr, 0);
    }
    if (cmds -> outfile) {
        if ((wrtr = open (cmds -> outfile, O_RDONLY, 0777)) > 0) {
            if (rdr != -1 && equal_files (rdr, wrtr)) {
                fprintf (stderr, "xsh: '%s' and '%s' are the same files\n", cmds -> infile, cmds -> outfile);
                dup2 (in, 0);
                return 1;
            }
        }
        if (cmds -> end_file) {
            wrtr = open (cmds -> outfile, O_WRONLY | O_APPEND | O_CREAT, 0777);
        } else {
            wrtr = open (cmds -> outfile, O_WRONLY | O_TRUNC  | O_CREAT, 0777);
        }
        out = dup (1);
        dup2 (wrtr, 1);
    }
	int argc = -1;
	int return_value;
	while (argv[++argc]) { ; }
	if (!strcmp(argv[0], "cd")) {


		switch (argc) {
		case 1:
			chdir(getenv("HOME"));
			return 0;
			break;
		case 2:
			return_value = chdir(argv[1]);
			if (return_value) {
				fprintf(stderr, "xsh: cd: %s: No such file or directory", argv[1]);
				exit(errno);
			}
			return 0;
			break;
		default:
			fprintf(stderr, "xsh: cd: too many arguments");
			return 1;
			break;
		}
	} else if (!strcmp(argv[0], "exit")) {
		exit(1);
	} else {
		if (!fork()) {
		    if (!cmds -> subshell) {


				if (!strcmp(argv[0], "pwd")) {
					switch (argc) {
					case 1:
						printf("%s\n", getcwd(NULL, 0));
						exit(0);
						break;
					default:
						fprintf(stderr, "xsh: pwd: too many arguments");
						exit(1);
						break;
					}
				}
		        execv (argv[0], argv);
		        execvp (argv[0], argv);
		        fprintf (stderr, "xsh: %s: command not found\n", argv[0]);
		        exit (errno);
		    }
		    char *argv_sh[3], *line;
		    argv_sh[0] = "./new_shell";
		    if ((cmds -> argv[0])[0] != '(' ||
		        (cmds -> argv[0])[str_len (cmds -> argv[0]) - 1] != ')') {
		            fprintf (stderr, "xsh: syntax error near %s\n", cmds -> argv[0]);
		            exit (1);
		    }
		    line = (char *) (cmds -> argv[0] + 1);
		    line [str_len (line) - 1] = '\0';
		    argv_sh[1] = line;
		    argv_sh[2] = NULL;
		    execv ("./new_shell", argv_sh);
		    exit (errno);
		} else {

		    wait (&status);

		    if (rdr != -1) {
		        close (rdr);
		        dup2 (in, 0);
		        close (in);
		    }
		    if (wrtr != -1) {
		        close (wrtr);
		        dup2 (out, 1);
		        close (out);
		    }
		    return (WIFEXITED (status))? WEXITSTATUS (status) : -100500;
		}
	}
}
