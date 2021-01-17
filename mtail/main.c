#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct lines {
    char *line;
    struct lines *next_line;
} lines;

typedef struct head_line {
    int number;
    int max;
    lines *first_line;
} head_line;

head_line *insert_line (
    char *line,
    head_line *hl)
{
    lines *tmp;

    if (hl -> number == hl -> max && hl -> max != 0) {
        tmp = hl -> first_line;
        hl -> first_line = tmp -> next_line;
        if (tmp -> line) {
            free (tmp -> line);
        }
        free (tmp);
    } else if (hl -> max == 0) {
        return hl;
    } else {
        (hl -> number)++;
    }
    tmp = hl -> first_line;
    if (!tmp) {
        tmp = malloc (sizeof (lines));
        tmp -> line = line;
        tmp -> next_line = NULL;
        hl -> first_line = tmp;
    } else {
        while (tmp -> next_line != NULL) {
            tmp = tmp -> next_line;
        }
        tmp -> next_line = malloc (sizeof (lines));
        tmp -> next_line -> line = line;
        tmp -> next_line -> next_line = NULL;
    }
    return hl;
}

head_line *free_head (
    head_line *hl)
{
    lines *deleter, *tmp = hl -> first_line;

    while (tmp) {
        deleter = tmp;
        tmp = tmp -> next_line;
        if (deleter -> line) {
            free (deleter -> line);
        }
        free (deleter);
    }
    free (hl);
    return NULL;
}

void print_lines (
    head_line *hl)
{
    lines *tmp = hl -> first_line;
    while (tmp) {
        printf ("%s", tmp -> line);
        tmp = tmp -> next_line;
    }
}

char *get_line (int);
void tail_handler_fd (int, const char **, int);
void tail_handler_input (int);

int main (int argc, const char **argv)
{
    char *line = NULL;
    int number, fd;

    if (argc > 3) {
        fprintf (stderr, "mtail: too many arguments\n");
        return 0;
    } else if (argc == 3) {
        if (sscanf (argv[1], "-%d", &number) == 0) {
            if (sscanf (argv[1], "+%d", &number) == 0) {
                fprintf (stderr, "mtail: '%s' is not a parameter\n", argv[1]);
                return 0;
            } else {
                if (number < 0) {
                    fprintf (stderr, "mtail: syntax error near '+%d'\n", number);
                    return 1;
                }
                if (number == 0) {
                    return 0;
                }
                if ((fd = open (argv[2], O_RDONLY, 0777)) < 0) {
                    fprintf (stderr, "mtail: '%s': No such file\n", argv[2]);
                    return 0;
                }
                int flag = 0;

                for (int i = 1; i < number; i++) {
                    if ((line = get_line (fd))[0] == '\0') {
                        flag = 1;
                        break;
                    }
                    free (line);
                }
                if (flag) {
                    free (line);
                    close (fd);
                    return 0;
                } else {
                    while (1) {
                        if ((line = get_line (fd))[0] == '\0') {
                            break;
                        } else {
                            printf("%s", line);
                            free (line);
                        }
                    }
                    free (line);
                    close (fd);
                    return 0;
                }
            }
        } else {
            tail_handler_fd (number, argv, 2);
        }
    } else if (argc == 2) {
        if (sscanf (argv[1], "-%d", &number) > 0) {
            tail_handler_input (number);
        } else if (sscanf (argv[1], "+%d", &number) > 0) {
            if (number < 0) {
                fprintf (stderr, "mtail: syntax error near '+%d'\n", number);
                return 1;
            }
            if (number == 0) {
                return 0;
            }

            int flag = 0;

            for (int i = 1; i < number; i++) {
                if ((line = get_line (0))[0] == '\0') {
                    flag = 1;
                    break;
                }
                free (line);
            }
            if (flag) {
                free (line);
                return 0;
            } else {
                while (1) {
                    if ((line = get_line (0))[0] == '\0') {
                        break;
                    } else {
                        printf("%s", line);
                        free (line);
                    }
                }
                free (line);
                return 0;
            }
        } else {
            tail_handler_fd (10, argv, 1);
        }
    } else if (argc == 1) {
        tail_handler_input (10);
    }
    return 0;
}

char *get_line (
    int fd)
{
    char tmp;
    int i = 0;
    char *line = NULL;

    while (read (fd, &tmp, 1)) {
        line = realloc (line, i + 1);
        line[i] = tmp;
        i++;
        if (tmp == '\n') {
            break;
        }
    }
    line = realloc (line, i + 1);
    line[i] = '\0';
    return line;
}

void tail_handler_fd (
    int number,
    const char **argv,
    int param)
{
    if (number < 0) {
        fprintf (stderr, "mtail: syntax error near '-%d'\n", number);
        return;
    }

    int fd;
    head_line *hl;
    char *line = NULL;

    hl = malloc (sizeof (head_line));
    hl -> number = 0;
    hl -> max = number;
    hl -> first_line = NULL;
    if ((fd = open (argv[param], O_RDONLY, 0777)) < 0) {
        fprintf (stderr, "mtail: '%s': No such file\n", argv[1]);
    } else {
        while ((line = get_line (fd))[0] != '\0') {
            hl = insert_line (line, hl);
        }
        free (line);
        print_lines (hl);
        free_head (hl);
    }
    close (fd);
}

void tail_handler_input (
    int number)
{
    if (number < 0) {
        fprintf (stderr, "mtail: syntax error near '-%d'", number);
        return;
    }
    if (!number) {
        return;
    }
    char *line = NULL;
    head_line *hl;

    hl = malloc (sizeof (head_line));
    hl -> number = 0;
    hl -> max = number;
    hl -> first_line = NULL;

    while ((line = get_line (0))[0] != '\0') {
        hl = insert_line (line, hl);
    }
    print_lines (hl);
    free_head (hl);
}
