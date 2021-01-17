#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

int equal_files (
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

int main(int argc, char **argv)
{
    int fd1, fd2;
    char tmp;

    if (argc != 3) {
        fprintf(stderr, "mcp: there should be two arguments\n");
        return 0;
    }

    if ((fd1 = open (argv[1], O_RDONLY, 0777)) < 0) {
        fprintf(stderr, "mcp: cannot stat '%s': no such file\n", argv[1]);
        return 0;
    }

    if ((fd2 = open (argv[2], O_RDONLY, 0777)) > 0) {
        if (equal_files (fd1, fd2)) {
            fprintf (stderr, "mcp: '%s' and '%s' are the same files\n", argv[1], argv[2]);
            return 0;
        }
        fd2 = open (argv[2], O_WRONLY|O_TRUNC, 0777);
    } else {
        fd2 = open (argv[2], O_WRONLY|O_CREAT, 0777);
    }

    while (read (fd1, &tmp, 1)) {
        write (fd2, &tmp, 1);
    }

    close (fd1);
    close (fd2);
    return 0;
}
