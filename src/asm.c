/*
 * Author:              Chukkala Dinesh
 * Roll No:             2001CS22
 * Web-mail:            chukkala_2001cs22@iitp.ac.in
 * Repository (Github): https://github.com/HumanChwan/CS322-SIMPLE-ASM
 * File name:           asm.c
 *
 * DECLARATION:
 * I hereby confirm that this assignment and the work presented in it is
 * entirely my own. I agree that the present work may be verified with
 * an anti-plagiarism software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool unsigned short
#define true 1
#define false 0

void print_usage() {
    fprintf(stderr, "Usage: asm [OPTIONS]... [FILE].asm\n");
    fprintf(stderr, "Try `asm -h` for more information\n");
}

void print_help() {
    printf("Usage: asm [OPTIONS]... [FILE].asm\n");
    printf("Reads MIPS assembly code and outputs .o .log .list files\n");
    printf("Example: `asm -o hello_world.asm\n\n");
    printf("Options:\n");
    printf("\t-l\t\tDon't output list file\n");
    printf("\t-L\t\tDon't output log file\n");
    printf("\t-o\t\tDon't output object file\n");
    printf("\t-h\t\tPrint this help\n");
}

typedef struct {
    bool log_file, list_file, obj_file;
    char* base_file_name;
} OPTIONS;

void initialise(OPTIONS* opt) {
    opt->list_file = opt->log_file = opt->obj_file = true;
}

void destruct(OPTIONS* opt) {
    free(opt->base_file_name);
}

bool enable_opt(OPTIONS* opt, char* arg) {
    if (strcmp(arg, "-l") == 0)
        opt->list_file = false;
    else if (strcmp(arg, "-o") == 0)
        opt->obj_file = false;
    else if (strcmp(arg, "-L") == 0)
        opt->log_file = false;
    else
        return false;
    return true;
}

bool check_if_file_exists_and_set(OPTIONS* opt, char* arg) {
    FILE* fp = fopen(arg, "r");
    int i, len;

    if (fp == NULL)
        return false;

    fclose(fp);
    len = strlen(arg);
    if (len < 4)
        return false;

    opt->base_file_name = calloc(sizeof(char), len - 3);

    for (i = 0; i < len - 4; ++i)
        opt->base_file_name[i] = arg[i];
    opt->base_file_name[i] = '\0';

    return true;
}

int main(int argc, char** argv) {
    OPTIONS opt;
    initialise(&opt);

    if (argc == 1 || argc > 5) {
        print_usage();
        return -1;
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0) {
            print_help();
            return 0;
        } else if (!check_if_file_exists_and_set(&opt, argv[1])) {
            print_usage();
            return -1;
        }
    } else {
        int i;
        for (i = 1; i < argc - 1; ++i) {
            if (!enable_opt(&opt, argv[i])) {
                print_usage();
                return -1;
            }
        }

        if (!check_if_file_exists_and_set(&opt, argv[i])) {
            print_usage();
            return -1;
        }
    }

    https://github.com/HumanChwan/CS322-SIMPLE-ASM

    destruct(&opt);
    return 0;
}