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

typedef enum
{
    ER_LABEL,
    ER_MNEMONIC,
    ER_INTEGER,
    ER_MISSING_OPERAND,
    ER_MORE_OPERAND,
    ER_BRANCH,
    ER_UNUSED_LABEL
} STATUS_CODE;

#define LABEL_ER "Label should of the format (regex): /[a-zA-Z]\\w*/"
#define MNEMONIC_ER                                                                                \
    "Mnemonics/isa should follow the given set of instructions. Find out more by `emu -isa`."
#define INTEGER_ER                                                                                 \
    ("Integers can be represented in decimal(0-9)[format: /(0|[1-9]\\d*)/, hex(0-F)[format:"       \
     " /0x[0-9a-fA-F]+/] or octal(0-7)[format: /0[0-7]+/] format.")
#define MISSING_ER "Was expecting an operand"
#define MORE_OPERAND_ER "Found more operands, than mnemonic requires"
#define BRANCH_ER "Unnecessary branching found, offset is 0"
#define UNUSED_LABEL "Unused label found"

const char *LOG_MESSAGES[] = {LABEL_ER,        MNEMONIC_ER, INTEGER_ER,  MISSING_ER,
                              MORE_OPERAND_ER, BRANCH_ER,   UNUSED_LABEL};

typedef struct STDERR_MESSAGE
{
    bool is_error; /* false: warning, true: error */
    int line_number;
    STATUS_CODE status_code;
    char *message;
    struct STDERR_MESSAGE *NEXT;
} STDERR_MESSAGE;

typedef struct
{
    STDERR_MESSAGE *head, *tail;
} STDERR_MESSAGE_LIST;

typedef struct
{
    char *identifier;
    int program_address;
    bool referenced;
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
    label->referenced = false;

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

void add_new_stderr_message(STDERR_MESSAGE_LIST *list, bool is_error, int line_number,
                            STATUS_CODE status_code, const char *message)
{
    STDERR_MESSAGE *node = malloc(sizeof(STDERR_MESSAGE));
    node->is_error = is_error;
    node->line_number = line_number;
    node->message = malloc(strlen(message) + 1);
    node->status_code = status_code;
    strcpy(node->message, message);
    node->NEXT = NULL;

    if (!list->head)
        list->head = node;
    else
        list->tail->NEXT = node;

    list->tail = node;
}

void destruct_stderr_message(STDERR_MESSAGE_LIST *list)
{
    STDERR_MESSAGE *iter = list->head;
    while (iter != NULL)
    {
        STDERR_MESSAGE *temp = iter;
        iter = iter->NEXT;
        free(temp);
    }
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
    {"ldc", 0, true, false},    {"adc", 1, true, false},  {"ldl", 2, true, false},
    {"stl", 3, true, false},    {"ldnl", 4, true, false}, {"stnl", 5, true, false},
    {"add", 6, false, false},   {"sub", 7, false, false}, {"shl", 8, false, false},
    {"shr", 9, false, false},   {"adj", 10, true, false}, {"a2sp", 11, false, false},
    {"sp2a", 12, false, false}, {"call", 13, true, true}, {"return", 14, false, false},
    {"brz", 15, true, true},    {"brlz", 16, true, true}, {"br", 17, true, true},
    {"HALT", 18, false, false}, {"SET", -2, true, false}, {"data", -1, true, false},
};
const int MNEMONIC_TYPES = 21;

void print_usage(void)
{
    fprintf(stderr, "Usage: asm [OPTIONS]... [FILE].asm\n"
                    "Try `asm -help` for more information\n");
}

void print_help(void)
{
    printf("Usage: asm [OPTIONS]... [FILE].asm\n"
           "Reads MIPS assembly code and outputs .o .log .lst files\n"
           "Example: `asm hello_world.asm`\n\n"
           "Options:\n"
           "\t-l\t\tDon't output list file\n"
           "\t-L\t\tDon't output log file\n"
           "\t-o\t\tDon't output object file\n"
           "\t-help\t\tPrint this help\n");
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

bool has_character(const char *str, char character)
{
    int i;
    for (i = 0; str[i] != '\0'; ++i)
        if (str[i] == character)
            return true;

    return false;
}

bool is_valid_number(const char *str)
{
    int i = 0, type = 0, len;
    /**
     * @type:
     * 0: Decimal (base 10)
     * 1: Octal (base 8)
     * 2: Hexadecimal (base 16)
     */
    /* Special Case: 0 */
    if (strcmp(str, "0") == 0)
        return true;

    if (str[0] == '-' || str[0] == '+')
        i++;

    len = strlen(str);
    if (len == i)
        return false;

    if (str[i] == '0')
    {
        i++;
        if (i < len && str[i] == 'x')
            i++, type = 2;
        else
            type = 1;
    }
    else
        type = 0;

    if (i == len)
        return false;

    for (; str[i] != '\0'; ++i)
    {
        if (type == 0 && !isdigit(str[i]))
            return false;
        else if (type == 1 && !('0' <= str[i] && str[i] < '8'))
            return false;
        else if (type == 2 && !isxdigit(str[i]))
            return false;
    }
    return true;
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

void initialise_log_and_list_file(OPTIONS *opt)
{
    FILE *fp_list = fopen(opt->list_filename, "w"), *fp_log = fopen(opt->log_filename, "w");

    fprintf(fp_list, "; This is the Listing file for: %s\n", opt->asm_filename);
    fprintf(fp_log, "; This is the Log file for: %s\n", opt->asm_filename);

    fclose(fp_list);
    fclose(fp_log);
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
    opt->list_filename = malloc(sizeof(char) * (len + 1));

    opt->obj_filename = malloc(sizeof(char) * (len - 1));

    strcpy(opt->asm_filename, arg);

    strncpy(opt->log_filename, arg, len - 4);
    strcpy(opt->log_filename + len - 4, ".log");

    strncpy(opt->list_filename, arg, len - 4);
    strcpy(opt->list_filename + len - 4, ".lst");

    strncpy(opt->obj_filename, arg, len - 4);
    strcpy(opt->obj_filename + len - 4, ".o");

    return true;
}

bool valid_label_name(char *label)
{
    int len;
    int i = 1;

    if (label == NULL)
        return false;

    len = strlen(label);

    if (!isalpha(label[0]) && label[0] != '_')
        return false;

    for (i = 1; i < len; ++i)
        if (!isalnum(label[i]) && label[i] != '_')
            return false;
    return true;
}

void list_and_form_obj(OPTIONS *opt, LABEL_LIST *label_list, STDERR_MESSAGE_LIST *stderr_list,
                       bool first_pass)
{
    FILE *fp, *fp_list, *fp_log, *fp_obj;
    char buffer[100], code_line[100];
    char *read, *label = NULL, *instruction;

    MNEMONIC *oper;
    char *oper_as_str, *operand_as_str;
    int PC = 0, line_number = 0;

    fp = fopen(opt->asm_filename, "r");

    fp_list = fopen(opt->list_filename, "a");
    fp_log = fopen(opt->log_filename, "a");
    fp_obj = fopen(opt->obj_filename, "wb");

    if (fp == NULL)
    {
        fprintf(stderr, "\033[0;31m[ERROR]: FATAL ERROR\033[0;0m");
        fclose(fp_list);
        fclose(fp_log);
        fclose(fp_obj);
        exit(-1);
    }

    while (fgets(buffer, 100, fp) != NULL)
    {
        bool has_label;
        int memory_dump, len;

        line_number++;

        if (buffer[0] == '\0' || buffer[0] == '\n')
            continue;
        if (buffer[0] == ';')
            continue;

        len = strlen(buffer);
        if (buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';

        read = strtok(buffer, ";");
        trim_whitespace(read);

        strcpy(code_line, read);

        has_label = has_character(read, ':');

        if (has_label)
        {
            label = strtok(read, ":");
            instruction = strtok(NULL, ":");
        }
        else
            instruction = read, label = NULL;

        if (label != NULL)
        {
            if (!valid_label_name(label))
            {
                /* ERROR: Invalid label */
                if (!first_pass)
                {
                    char message[100] = {'\0'};
                    strcat(message, "Invalid label: ");
                    strcat(message, label);
                    add_new_stderr_message(stderr_list, true, line_number, ER_LABEL, message);
                }
                continue;
            }
            else if (first_pass)
            {
                /* label duplication checks and/or storing label address */
                LABEL *label_data = search_for_label(label_list, label);
                int val = PC;
                if (label_data != NULL)
                {
                    /* ERROR: found duplicate label identifier */
                    char message[100] = {'\0'};
                    strcat(message, "Found Duplicate labels: ");
                    strcat(message, label);
                    add_new_stderr_message(stderr_list, true, line_number, ER_LABEL, message);
                    continue;
                }

                /* Implementation for SET instruction */
                if (instruction != NULL)
                {
                    /* SET instruction */
                    /**
                     * label: SET value
                     * this statement can be understood as:
                     * label `label` pointing to the the memory location `value`
                     */
                    trim_whitespace(instruction);
                    convert_space_words_to_space(instruction);

                    if (strcmp(strtok(instruction, " "), "SET") == 0)
                    {
                        operand_as_str = strtok(NULL, " ");
                        if (operand_as_str == NULL || !is_valid_number(operand_as_str))
                        {
                            /* ERROR: SET instruction expects a valid integer */
                            char message[100] = {'\0'};
                            strcat(message, "SET instruction expects integer format as operand: ");
                            strcat(message, operand_as_str);
                            add_new_stderr_message(stderr_list, true, line_number, ER_INTEGER,
                                                   message);
                            continue;
                        }

                        PC--;
                        val = strtol(operand_as_str, NULL, 0);
                    }
                }

                add_new_label(label_list, label, val);
            }
        }

        /* If first pass then skipping instruction checking and listing */
        if (first_pass)
        {
            if (instruction != NULL)
                PC++;
            continue;
        }

        /* Following area of code is executed only in the second pass */

        /**
         * Printing the PC in the listing file
         * @format:
         *      PC(%08X)   memory_dump  code_line
         * @example:
         *      00000001    00001200    ldc 0x12
         */

        fprintf(fp_list, "%08X\t", PC);

        if (instruction != NULL)
        {
            PC++;
            trim_whitespace(instruction);

            /* format of instruction: /\w[\w\s]*\w/ */
            convert_space_words_to_space(instruction);
            /* format of instruction: /\w(\w+ )*\w/ */

            oper_as_str = strtok(instruction, " ");
            operand_as_str = strtok(NULL, " ");

            oper = get_mnemonic(oper_as_str);
            if (oper == NULL)
            {
                /* ERROR: mnemonic not found */
                char message[100] = {'\0'};
                strcat(message, "Mnemonic not found: ");
                strcat(message, oper_as_str);
                add_new_stderr_message(stderr_list, true, line_number, ER_MNEMONIC, message);
                continue;
            }

            if (!oper->has_operand && operand_as_str != NULL)
            {
                /* ERROR: found operand for isolated mnemonic */
                char message[100] = {'\0'};
                strcat(message, "Found operand(s) for isolated mnemonic: ");
                strcat(message, code_line);
                add_new_stderr_message(stderr_list, true, line_number, ER_MORE_OPERAND, message);
                continue;
            }
            else if (oper->has_operand && operand_as_str == NULL)
            {
                /* ERROR: Expected operand, but did not find any */
                char message[100] = {'\0'};
                strcat(message, "Expected an operand, found none: ");
                strcat(message, code_line);
                add_new_stderr_message(stderr_list, true, line_number, ER_MISSING_OPERAND, message);
                continue;
            }

            memory_dump = oper->OPCODE;

            if (oper->has_operand)
            {
                LABEL *label = search_for_label(label_list, operand_as_str);

                if (label != NULL)
                    label->referenced = true;

                if (oper->relative_calc)
                {
                    int offset;
                    if (label == NULL)
                    {
                        /* ERROR: undefined reference to a label */
                        char message[100] = {'\0'};
                        strcat(message, "Could not find any such label: ");
                        strcat(message, operand_as_str);
                        add_new_stderr_message(stderr_list, true, line_number, ER_LABEL, message);
                        /** ERROR("Find out more details in file: %s\n\n",
                         * opt->log_filename); */
                        continue;
                    }
                    offset = ((label->program_address - PC) << 8);

                    if (offset == 0)
                    {
                        /* WARNING: unnecessary branching */
                        char message[100] = {'\0'};
                        strcat(message, "Branching with 0 offset: ");
                        strcat(message, code_line);
                        add_new_stderr_message(stderr_list, false, line_number, ER_BRANCH, message);
                    }
                    memory_dump += offset;
                }
                else
                {
                    int operand;
                    if (!label && !is_valid_number(operand_as_str))
                    {
                        /* Non integer operand found where integer expected */
                        char message[100] = {'\0'};
                        strcat(message, "Expected integer format, found: ");
                        strcat(message, operand_as_str);
                        add_new_stderr_message(stderr_list, true, line_number, ER_INTEGER, message);
                        continue;
                    }
                    if (label)
                        operand = label->program_address;
                    else
                        /* Auto detect hex, octal or decimal */
                        operand = strtol(operand_as_str, NULL, 0);

                    memory_dump += (operand << 8);

                    /* data and SET handling */
                    if (oper->OPCODE == -1)
                    {
                        /* data instruction */
                        /**
                         * label: data value
                         * this statement can be understood as:
                         * label `label` pointing to some location in the instruction memory
                         * And at that memory address `value` is stored
                         */
                        memory_dump = operand;
                    }
                    else if (oper->OPCODE == -2)
                    {
                        /* Should be implemented and checked in the first pass itself */

                        /* Should be just ignored and not mentioned in the obj file */
                        /* PC should not be incremented in this case */
                        fprintf(fp_list, "%*c\t%s\n", 8, ' ', code_line);
                        PC--;
                        continue;
                    }
                }
            }

            fprintf(fp_list, "%08X\t%s\n", memory_dump, code_line);
            fwrite(&memory_dump, 4, 1, fp_obj);
        }
        else
            fprintf(fp_list, "%*c\t%s\n", 8, ' ', code_line);
    }

    fclose(fp);
    fclose(fp_list);
    fclose(fp_obj);
    fclose(fp_log);
}

void check_for_unused_label(LABEL_LIST *list, STDERR_MESSAGE_LIST *stderr_list)
{
    LABEL_NODE *iter;
    for (iter = list->HEAD; iter != NULL; iter = iter->NEXT)
    {
        if (!iter->label->referenced)
        {
            char message[100] = {'\0'};
            strcat(message, "Found unused label: ");
            strcat(message, iter->label->identifier);
            add_new_stderr_message(stderr_list, false, iter->label->program_address,
                                   ER_UNUSED_LABEL, message);
        }
    }
}

void err_warn_log(STDERR_MESSAGE_LIST *stderr_list, const char *log_filename)
{
    FILE *fp_log = fopen(log_filename, "a");
    STDERR_MESSAGE *iter;
    
    for (iter = stderr_list->head; iter != NULL; iter = iter->NEXT)
    {
        if (iter->is_error)
        {
            fprintf(stderr,
                    "\033[0;31m[ERROR]: \033[0;0m%d|\t%s\n"
                    "\033[0;31m[ERROR]: \033[0;0m"
                    "Find out more in the log file: \033[0;34m%s\033[0;0m\n",
                    iter->line_number, iter->message, log_filename);

            fprintf(fp_log,
                    "[ERROR]: %d|\t%s\n"
                    "[ERROR]: %s\n",
                    iter->line_number, iter->message, LOG_MESSAGES[iter->status_code]);
        }
        else
        {
            fprintf(stderr,
                    "\033[0;33m[WARNING]: \033[0;0m%d|\t%s\n"
                    "\033[0;33m[WARNING]: \033[0;0m"
                    "Find out more in the log file: \033[0;34m%s\033[0;0m\n",
                    iter->line_number, iter->message, log_filename);

            fprintf(fp_log,
                    "[WARNING]: %d|\t%s\n"
                    "[WARNING]: %s\n",
                    iter->line_number, iter->message, LOG_MESSAGES[iter->status_code]);
        }
    }

    fclose(fp_log);
}

void destroy_and_exit(OPTIONS *opt, LABEL_LIST *list, STDERR_MESSAGE_LIST *stderr_list, int status)
{
    destruct_list(list);
    destruct(opt);
    destruct_stderr_message(stderr_list);

    exit(status);
}

int main(int argc, char **argv)
{
    OPTIONS opt;
    LABEL_LIST label_list;
    STDERR_MESSAGE_LIST stderr_list = {NULL, NULL};

    initialise(&opt);
    initialise_list(&label_list);

    if (argc == 1 || argc > 5)
    {
        print_usage();
        destroy_and_exit(&opt, &label_list, &stderr_list, -1);
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-help") == 0)
        {
            print_help();
            destroy_and_exit(&opt, &label_list, &stderr_list, 0);
        }
        else if (!check_if_file_exists_and_set(&opt, argv[1]))
        {
            print_usage();
            destroy_and_exit(&opt, &label_list, &stderr_list, -1);
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
                destroy_and_exit(&opt, &label_list, &stderr_list, -1);
            }
        }

        if (!check_if_file_exists_and_set(&opt, argv[i]))
        {
            print_usage();
            destroy_and_exit(&opt, &label_list, &stderr_list, -1);
        }
    }

    initialise_log_and_list_file(&opt);

    /* passing through the procedure to read and form object/listing/log file twice */
    list_and_form_obj(&opt, &label_list, &stderr_list, true);
    list_and_form_obj(&opt, &label_list, &stderr_list, false);

    check_for_unused_label(&label_list, &stderr_list);

    err_warn_log(&stderr_list, opt.log_filename);

    if (!opt.list_file)
        remove(opt.list_filename);
    if (!opt.log_file)
        remove(opt.log_filename);
    if (!opt.obj_file)
        remove(opt.obj_filename);

    destroy_and_exit(&opt, &label_list, &stderr_list, 0);
    return 0;
}
