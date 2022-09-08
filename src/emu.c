/*
 *
 * Author:              Chukkala Dinesh
 * Roll No:             2001CS22
 * Web-mail:            chukkala_2001cs22@iitp.ac.in
 * Repository (Github): https://github.com/HumanChwan/CS322-SIMPLE-ASM
 * File name:           emu.c
 *
 * DECLARATION:
 * I hereby confirm that this assignment and the work presented in it is
 * entirely my own. I agree that the present work may be verified with
 * an anti-plagiarism software.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool unsigned short
#define true 1
#define false 0

typedef struct
{
    bool trace;
    bool mem_dump_before;
    bool mem_dump_after;
    bool isa;
    char *obj_filename;
} OPTIONS;

void print_usage()
{
    fprintf(stderr, "Usage: emu [OPTIONS]... [FILE].o\n");
    fprintf(stderr, "Try `emu -help` for more information\n");
}

void print_help()
{
    printf("Usage: emu [OPTIONS]... [FILE].o\n");
    printf("Reads object file and emulates execution.\n");
    printf("Example: `emu -trace hello_world.o`\n\n");
    printf("Options:\n");
    printf("\t-trace\t\tShows instruction trace\n");
    printf("\t-before\t\tPrint memory dump, before execution\n");
    printf("\t-after\t\tPrint memory dump, after execution\n");
    printf("\t-isa\t\tPrint this help\n");
    printf("\t-help\t\tPrint this help\n");
}

void enable_opt(OPTIONS *opt, const char *arg)
{
    if (strcmp(arg, "-isa") == 0)
        opt->isa = true;
    else if (strcmp(arg, "-before") == 0)
        opt->mem_dump_before = true;
    else if (strcmp(arg, "-after") == 0)
        opt->mem_dump_after = true;
    else if (strcmp(arg, "-trace") == 0)
        opt->trace = true;
}

bool check_if_file_exists_and_set(OPTIONS *opt, const char *arg)
{
    FILE *fp = fopen(arg, "rb");
    int len;

    if (fp == NULL)
        return false;

    fclose(fp);

    /* `arg` can now be assumed to be a valid file  */
    len = strlen(arg);

    if (len < 2)
        return false;
    /* `arg` can now be assumed to have atleast 2 characters  */

    if (strcmp(arg + len - 2, ".o") != 0)
        return false;
    /* `arg` can now be assumed to have an extension of ".o" */

    opt->obj_filename = malloc(len + 1);
    strcpy(opt->obj_filename, arg);

    return true;
}

void destruct_and_exit(OPTIONS *opt, int status_code)
{
    free(opt->obj_filename);
    exit(status_code);
}

int main(int argc, char **argv)
{
    OPTIONS opt = {false, false, false, false, NULL};

    if (argc == 1)
    {
        print_usage();
        destruct_and_exit(&opt, -1);
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-help") == 0)
        {
            print_help();
            destruct_and_exit(&opt, 0);
        }
        print_usage();
        destruct_and_exit(&opt, -1);
    }
    else
    {
        int i;
        for (i = 1; i < argc - 1; ++i)
            enable_opt(&opt, argv[i]);

        if (!check_if_file_exists_and_set(&opt, argv[i]))
        {
            print_usage();
            destruct_and_exit(&opt, -1);
        }
    }

    destruct_and_exit(&opt, 0);
    return 0;
}