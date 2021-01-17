#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void counter (int, int*, int*, int*);

//int tmpCreateFile()
//{
//    int fd = creat("tmp.tmp", 0);
//    char c;
//
///*
//    c = '\0';
//    write(fd, &c, 1);
//    c = 'a';
//    write(fd, &c, 1);
//    c = '\0';
//    write(fd, &c, 1);
//    c = 'h';
//    write(fd, &c, 1);
//*/
////    c = '\r'; //'\t' || c == '\v' || c == '\f' || c == '\r'
////    write(fd, &c, 1);
////    c = 127;//'\0';
////    write(fd, &c, 1);
////
//////    for (int i = 0; i < 31; i++) {
//////        write (fd, &i, 1);
//////    }
////    c = ' ';
////    write(fd, &c, 1);
///*
//    c = 'b';
//    write(fd, &c, 1);
//    c = '\0';
//    write(fd, &c, 1);
//    c = 'a';
//    write(fd, &c, 1);
//    c = '\0';
//    write(fd, &c, 1);
//*/
//    close(fd);
//
//    return 0;
//}
//
////int tmp_local = tmpCreateFile();

int main (int argc, const char **argv)
{
//    tmpCreateFile();
    int   symbols = 0,   words = 0,   lines = 0;
    int alsymbols = 0, alwords = 0, allines = 0;
    int fd;

    if (argc < 2) {
        counter (0, &symbols, &words, &lines);
        printf ("\t%d\t%d\t%d\n", lines, words, symbols);
    } else if (argc == 2) {
        if ((fd = open (argv[1], O_RDONLY, 0777)) < 0) {
            fprintf (stderr, "mwc: %s: No such file\n", argv[1]);
            return 1;
        }
        counter (fd, &symbols, &words, &lines);
        printf ("\t%d\t%d\t%d\t%s\n", lines, words, symbols, argv[1]);
        close (fd);
    } else {
        int i = 1;

        while (--argc >= 1) {
            symbols = words = lines = 0;
            if ((fd = open (argv[i], O_RDONLY, 0777)) < 0) {
                fprintf (stderr, "mwc: %s: No such file\n", argv[i]);
                return 1;
            }
            counter (fd, &symbols, &words, &lines);
            printf ("\t%d\t%d\t%d\t%s\n", lines, words, symbols, argv[i]);
            alsymbols += symbols;
            alwords += words;
            allines += lines;
            i++;
            close (fd);
        }
        printf ("\t%d\t%d\t%d\ttotal\n", allines, alwords, alsymbols);
    }

    return 0;
}

int is_space (
    char c)
{
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\n' || c == '\r';
}

int is_space_without_line (
    char c)
{
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}
//int get_word (
//	int fd,
//    int *symbols,
//	int *words,
//	int *lines)
//{
//    char c;
//    int readed;
//    while ((readed = read (fd, &c, 1))) {
//
//		if (isalpha (c)) {
//			(*words)++;
//		}
//        (*symbols)++;
//		if (c == '\n') {
//			(*lines)++;
//		}
//		for (; readed; readed = read (fd, &c, 1)) {
//            (*symbols)++;
//            if (c == '\n') {
//                (*lines)++;
//            }
//            if (!isalpha (c)) {
//                break;
//            }
//		}
//    }
//    return readed;
//}

void counter (
    int fd,
    int *symbols,
    int *words,
    int *lines)
{
    char c;
    char prev = ' ';

    int flag = 0;

    int sp = 0, tab = 0, vert = 0, feed = 0, lin = 0, car = 0;
    while (read (fd, &c, 1)) {
        (*symbols)++;

        if (is_space (c)) {
            if (c == '\n') {
                (*lines)++;
            }
            if (!is_space (prev) && flag) {

                if (c == ' ') sp++;
                if (c == '\t') tab++;
                if (c == '\v') vert++;
                if (c == '\f') feed++;
                if (c == '\n') lin++;
                if (c == '\r') car++;
                (*words)++;
                flag = 0;
            }
        }
        if ((int) c >= 33 && c <= 126) {
            flag = 1;
        }
        prev = c;
    }
    if (!is_space (prev) && flag) {
        (*words)++;
    }
    //printf ("spaces %d, tabs %d, verts %d, feeds %d, lins %d, cars %d", sp, tab, vert, feed, lin, car);
}
