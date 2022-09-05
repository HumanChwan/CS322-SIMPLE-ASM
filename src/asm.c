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

#define ERROR(...)                                                                                 \
    fprintf(stderr, "\033[0;31mERROR: \033[0;0m");                                                 \
    fprintf(stderr, __VA_ARGS__);

#define WARNING(...)                                                                               \
    fprintf(stderr, "\033[0;33mWARNING: \033[0;0m");                                               \
    fprintf(stderr, __VA_ARGS__);

typedef struct
{
    bool log_file, list_file, obj_file;
    char *asm_filename, *log_filename, *list_filename, *obj_filename;
} OPTIONS;

typedef struct
{
    char OPER[7];
    int OPCODE;
    bool has_operand;
    bool relative_calc;
} MNEMONIC;

typedef struct
{
    char *identifier;
    int program_address;
} LABEL;

typedef struct LABEL_NODE
{
    LABEL *label;
    struct LABEL_NODE *NEXT;
} LABEL_NODE;

typedef struct
{
    LABEL_NODE *HEAD, *TAIL;
} LABEL_LIST;

void initialise_list(LABEL_LIST *label_list)
{
    label_list->HEAD = label_list->TAIL = NULL;
}

LABEL *create_label(const char *label_identifier, int address)
{
    LABEL *label = malloc(sizeof(LABEL));
    label->program_address = address;
    label->identifier = malloc(sizeof(char) * (strlen(label_identifier) + 1));
    strcpy(label->identifier, label_identifier);

    return label;
}

LABEL_NODE *create_label_node(const char *label_identifier, int address)
{
    LABEL_NODE *node = malloc(sizeof(LABEL_NODE));
    node->label = create_label(label_identifier, address);
    node->NEXT = NULL;

    return node;
}

void add_new_label(LABEL_LIST *list, const char *label_identifier, int address)
{
    LABEL_NODE *node = create_label_node(label_identifier, address);

    if (list->HEAD == NULL)
        list->HEAD = node;
    else
        list->TAIL->NEXT = node;

    list->TAIL = node;
}

/**
 * @brief: searches for a particular label
 *
 * @params: const char* label_identifier
 * @return: LABEL* if found else NULL
 *
 */
LABEL *search_for_label(LABEL_LIST *list, const char *label_identifier)
{
    LABEL_NODE *iter;
    for (iter = list->HEAD; iter != NULL; iter = iter->NEXT)
        if (strcmp(iter->label->identifier, label_identifier) == 0)
            return iter->label;

    return NULL;
}

void destruct_list(LABEL_LIST *list)
{
    LABEL_NODE *iter = list->HEAD;

    while (iter != NULL)
    {
        LABEL_NODE *temp = iter;
        iter = iter->NEXT;
        free(temp);
    }
}

/**
 * @brief: Mnemonic data below
 *
 * @mnemonic_schema: "mnemonic", opcode, accepts an operand, has to recalculated
 */
MNEMONIC mnemonics[] = {
    {"data", -1, true, false},    {"ldc", 0, true, false},    {"adc", 1, true, false},
    {"ldl", 2, true, false},      {"stl", 3, true, false},    {"ldnl", 4, true, false},
    {"stnl", 5, true, false},     {"add", 6, false, false},   {"sub", 7, false, false},
    {"shl", 8, false, false},     {"shr", 9, false, false},   {"adj", 10, true, false},
    {"a2sp", 11, false, false},   {"sp2a", 12, false, false}, {"call", 13, true, true},
    {"return", 14, false, false}, {"brz", 15, true, true},    {"brlz", 16, true, true},
    {"br", 17, true, true},       {"HALT", 18, false, false}, {"SET", 19, true, false},
};
const int MNEMONIC_TYPES = 20;

void print_usage()
{
    fprintf(stderr, "Usage: asm [OPTIONS]... [FILE].asm\n");
    fprintf(stderr, "Try `asm -h` for more information\n");
}

void print_help()
{
    printf("Usage: asm [OPTIONS]... [FILE].asm\n");
    printf("Reads MIPS assembly code and outputs .o .log .list files\n");
    printf("Example: `asm -o hello_world.asm\n\n");
    printf("Options:\n");
    printf("\t-l\t\tDon't output list file\n");
    printf("\t-L\t\tDon't output log file\n");
    printf("\t-o\t\tDon't output object file\n");
    printf("\t-h\t\tPrint this help\n");
}

void trim_whitespace(char *str)
{
    int len, i = 0, j = 0, k = 0;
    char *cpy;
    if (str == NULL)
        return;

    len = strlen(str);
    while (i < len && isspace(str[i]))
        i++;
    while (i < len - j - 1 && isspace(str[len - 1 - j]))
        j++;

    if (len - j - i == 0)
    {
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

void convert_space_words_to_space(char *str)
{
    char converted[100];
    int i, j;
    bool got_space = false;

    for (i = 0, j = 0; str[i] != '\0'; ++i)
    {
        if (isspace(str[i]))
            got_space = true;
        else
        {
            if (got_space)
                converted[j++] = ' ';
            got_space = false;

            converted[j++] = str[i];
        }
    }
    converted[j] = '\0';
    strcpy(str, converted);
}

/**
 * @brief: function to get mnemonic from ISA
 *
 * @params: const char* oper
 * @return: MNEMONIC* if found, otherwise NULL
 *
 */
MNEMONIC *get_mnemonic(const char *oper)
{
    int i;
    for (i = 0; i < MNEMONIC_TYPES; ++i)
    {
        if (strcmp(mnemonics[i].OPER, oper) == 0)
            return &mnemonics[i];
    }
    return NULL;
}

void initialise(OPTIONS *opt)
{
    opt->list_file = opt->log_file = opt->obj_file = true;
    opt->asm_filename = opt->list_filename = opt->log_filename = opt->obj_filename = NULL;
}

void destruct(OPTIONS *opt)
{
    if (opt->asm_filename != NULL)
        free(opt->asm_filename);
    if (opt->obj_filename != NULL)
        free(opt->obj_filename);
    if (opt->list_filename != NULL)
        free(opt->list_filename);
    if (opt->log_filename != NULL)
        free(opt->log_filename);
}

bool enable_opt(OPTIONS *opt, char *arg)
{
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

bool check_if_file_exists_and_set(OPTIONS *opt, char *arg)
{
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

    strncpy(opt->log_filename, arg, len - 4);
    strcpy(opt->log_filename + len - 4, ".log");

    strncpy(opt->list_filename, arg, len - 4);
    strcpy(opt->list_filename + len - 4, ".l");

    strncpy(opt->obj_filename, arg, len - 4);
    strcpy(opt->obj_filename + len - 4, ".o");

    return true;
}

bool valid_label_name(char *label)
{
    int len = strlen(label);
    int i = 1;

    if (!isalpha(label[0]) && label[0] != '_')
        return false;

    for (i = 1; i < len; ++i)
        if (!isalnum(label[i]) && label[i] != '_')
            return false;
    return true;
}

bool list_and_log_and_form_obj(OPTIONS *opt, LABEL_LIST *label_list, bool first_pass)
{
    FILE *fp, *fp_list, *fp_log, *fp_obj;
    char buffer[100], code_line[100];
    char *read, *label = NULL, *instruction;

    MNEMONIC *oper;
    char *oper_as_str, *operand_as_str;
    int operand;
    int PC = 0;

    fp = fopen(opt->asm_filename, "r");

    if (!first_pass)
    {
        fp_list = fopen(opt->list_filename, "w");
        fp_log = fopen(opt->log_filename, "w");
        fp_obj = fopen(opt->obj_filename, "wb");
    }

    if (fp == NULL)
    {
        fprintf(stderr, "FATAL ERROR");
        exit(-1);
    }

    while (fgets(buffer, 100, fp) != NULL)
    {
        bool has_label, has_opcode_list;
        int memory_dump;

        trim_whitespace(buffer);
        if (strlen(buffer) == 0)
            continue;

        if (buffer[0] == ';')
            continue;
        buffer[strlen(buffer) - 1] = '\0';
        read = strtok(buffer, ";");
        trim_whitespace(read);

        strcpy(code_line, read);

        has_label = (read[strlen(read) - 1] == ':');

        if (has_label)
        {
            label = strtok(read, ":");
            instruction = strtok(NULL, ":");
        }
        else
            instruction = read;

        if (label != NULL)
        {
            if (!valid_label_name(label))
            {
                fprintf(fp_log, "%08X\tInvalid label: |%s:|", PC, label);
                ERROR("Invalid label at %08X found in format: |%s:|\n", PC, label);
                return false;
            }
            else
            {
                /* label duplication checks and/or storing label address */
                LABEL *label_data = search_for_label(label_list, label);
                if (label_data != NULL)
                {
                    /* ERROR: found duplicate label identifier */
                    return false;
                }

                add_new_label(label_list, label, PC);
            }
        }

        /* If first pass then skipping instruction checking and listing */
        if (first_pass)
            continue;

        /* Following area of code is executed only in the second pass */

        /* printing the PC in the listing file */
        fprintf(fp_list, "%08X ", PC);

        if (instruction != NULL)
        {
            trim_whitespace(instruction);

            /* format of instruction: /[\w\s]*\w/ */
            convert_space_words_to_space(instruction);
            /* format of instruction: /(\w+ )*\w/ */

            oper_as_str = strtok(instruction, " ");
            operand_as_str = strtok(NULL, " ");

            oper = get_mnemonic(oper_as_str);
            if (oper == NULL)
            {
                /* ERROR: mnemonic not found */
                return false;
            }

            if (!oper->has_operand && operand_as_str != NULL)
            {
                /* ERROR: found operand for isolated mnemonic */
                return false;
            }
            else if (oper->has_operand && operand_as_str == NULL)
            {
                /* ERROR: Expected operand, but did not find any */
                return false;
            }

            PC++;
        }
        else
        {
            fprintf(fp_list, "%*c", 9, ' ');
        }

        fprintf(fp_list, "%s", code_line);
    }

    fclose(fp);
    if (!first_pass)
    {
        fclose(fp_list);
        fclose(fp_obj);
        fclose(fp_log);
    }

    return true;
}

void destroy_and_exit(OPTIONS *opt, LABEL_LIST *list, int status)
{
    destruct_list(list);
    destruct(opt);
    exit(status);
}

int main(int argc, char **argv)
{
    OPTIONS opt;
    LABEL_LIST label_list;

    initialise(&opt);
    initialise_list(&label_list);

    if (argc == 1 || argc > 5)
    {
        print_usage();
        destroy_and_exit(&opt, &label_list, -1);
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-h") == 0)
        {
            print_help();
            destroy_and_exit(&opt, &label_list, 0);
        }
        else if (!check_if_file_exists_and_set(&opt, argv[1]))
        {
            print_usage();
            destroy_and_exit(&opt, &label_list, -1);
        }
    }
    else
    {
        int i;
        for (i = 1; i < argc - 1; ++i)
        {
            if (!enable_opt(&opt, argv[i]))
            {
                print_usage();
                destroy_and_exit(&opt, &label_list, -1);
            }
        }

        if (!check_if_file_exists_and_set(&opt, argv[i]))
        {
            print_usage();
            destroy_and_exit(&opt, &label_list, -1);
        }
    }

    /* passing through the procedure to read and form object/listing/log file twice */
    if (!list_and_log_and_form_obj(&opt, &label_list, true) ||
        !list_and_log_and_form_obj(&opt, &label_list, false))
    {
        destroy_and_exit(&opt, &label_list, -1);
    }

    destroy_and_exit(&opt, &label_list, 0);
}