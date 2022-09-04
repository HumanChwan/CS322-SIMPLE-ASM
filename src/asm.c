/*
 *
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
 *
 */

#include <ctype.h>
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

void trim_whitespace(char *str) {
    int len, i = 0, j = 0, k = 0;
    char *cpy;
    if (str == NULL)
        return;

    len = strlen(str);
    while (i < len && (str[i] == ' ' || str[i] == '\t'))
        i++;
    while (i < len - j - 1 &&
           (str[len - 1 - j] == ' ' || str[len - 1 - j] == '\t'))
        j++;

    if (len - j - i == 0) {
        *str = '\0';
        return;
    }

    cpy = malloc(len - i - j + 1);
    for (k = 0; k < len - j - i; ++k)
        cpy[k] = str[i + k];
    cpy[k] = '\0';

    strcpy(str, cpy);
    free(cpy);
}

typedef struct {
    bool log_file, list_file, obj_file;
    char *asm_filename, *log_filename, *list_filename, *obj_filename;
} OPTIONS;

void initialise(OPTIONS *opt) {
    opt->list_file = opt->log_file = opt->obj_file = true;
}

void destruct(OPTIONS *opt) {
    free(opt->asm_filename);
    free(opt->obj_filename);
    free(opt->list_filename);
    free(opt->log_filename);
}

bool enable_opt(OPTIONS *opt, char *arg) {
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

bool check_if_file_exists_and_set(OPTIONS *opt, char *arg) {
    FILE *fp = fopen(arg, "r");
    int len;

    if (fp == NULL)
        return false;

    fclose(fp);

    len = strlen(arg);
    if (len < 4)
        return false;

    if (strcmp(arg + len - 4, ".asm") != 0)
        return false;

    opt->asm_filename = malloc(sizeof(char) * (len + 1));
    opt->log_filename = malloc(sizeof(char) * (len + 1));

    opt->list_filename = malloc(sizeof(char) * (len - 1));
    opt->obj_filename = malloc(sizeof(char) * (len - 1));

    strcpy(opt->asm_filename, arg);

    strcpy(opt->log_filename, arg);
    strcpy(opt->log_filename + len - 4, ".log");

    strcpy(opt->list_filename, arg);
    strcpy(opt->list_filename + len - 4, ".l");

    strcpy(opt->obj_filename, arg);
    strcpy(opt->obj_filename + len - 4, ".o");

    return true;
}

bool valid_label_name(char *label) {
    int len = strlen(label);
    int i = 1;

    if (!isalpha(label[0]) && label[0] != '_')
        return false;

    for (i = 1; i < len; ++i)
        if (!isalnum(label[i]) && label[i] != '_')
            return false;
    return true;
}

bool list_and_log(OPTIONS *opt, bool first_pass) {
    FILE *fp;
    char buffer[100];
    char *read, *label, *instruction;

    char oper[20];
    int operand;

    fp = fopen(opt->asm_filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "FATAL ERROR");
        exit(-1);
    }

    while (fgets(buffer, 100, fp) != NULL) {
        if (buffer[0] == ';')
            continue;
        buffer[strlen(buffer) - 1] = '\0';
        read = strtok(buffer, ";");
        trim_whitespace(read);

        label = strtok(read, ":");
        instruction = strtok(NULL, ":");

        if (instruction == NULL)
            instruction = label, label = NULL;

        if (label != NULL && !valid_label_name(label)) {
            return false;
        }

        trim_whitespace(instruction);
    }
    if (first_pass) {
        printf("first pass");
    }

    return true;
}

int main(int argc, char **argv) {
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

    printf("list:\t%s\n", opt.list_filename);
    printf("log:\t%s\n", opt.log_filename);
    printf("obj:\t%s\n", opt.obj_filename);
    printf("file:\t%s\n", opt.asm_filename);

    list_and_log(&opt, true);

    destruct(&opt);
    return 0;
}