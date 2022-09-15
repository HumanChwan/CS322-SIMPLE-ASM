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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool unsigned short
#define true 1
#define false 0

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define NORMAL "\033[0;0m"

/* Memory Size: 0x100000 (approx ~ 1e6) */
#define MEMORY_SIZE 0x100000

typedef struct
{
    bool trace;
    bool read_flag;
    bool write_flag;
    bool mem_dump_before;
    bool mem_dump_after;
    bool isa;
    char *obj_filename;
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
    unsigned int A;
    unsigned int B;
    int Program_Counter;
    int Stack_Pointer;
} REGISTERS;

typedef struct
{
    unsigned int raw[MEMORY_SIZE];
    int instruction_memory_size;
} MEMORY;

/**
 * @brief: Mnemonic data below
 * @to_be_observed: OPCODE is same as the index of the MNEMONIC except for the last 2
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
const int VALID_INSTRUCTION_MNEMONIC_TYPES = 19;

void print_usage(void)
{
    fprintf(stderr, "Usage: emu [OPTIONS]... [FILE].o\n"
                    "Try `emu -help` for more information\n");
}

void print_help(void)
{
    printf("Usage: emu [OPTIONS]... [FILE].o\n"
           "Reads object file and emulates execution.\n"
           "Example: `emu -trace hello_world.o`\n\n"
           "Options:\n"
           "\t-trace\t\tShows instruction trace\n"
           "\t-before\t\tPrint memory dump, before execution\n"
           "\t-after\t\tPrint memory dump, after execution\n"
           "\t-isa\t\tDisplay ISA\n"
           "\t-help\t\tPrint this help\n");
}

void print_isa(void)
{
    int i;
    printf("ISA For SIMPLE-ASM:\n\n");
    for (i = 0; i < MNEMONIC_TYPES; ++i)
    {
        printf("%02X:\t" YELLOW "%s" NORMAL "\t" BLUE "%s" NORMAL "\n",
               (mnemonics[i].OPCODE >= 0 ? mnemonics[i].OPCODE : 00), mnemonics[i].OPER,
               mnemonics[i].has_operand ? (mnemonics[i].relative_calc ? "label" : "value") : "\0");
    }
}

void memory_dump(REGISTERS *registers, MEMORY *memory, int L_limit, int R_Limit)
{
    int i;
    printf(RED "Memory Dump:" NORMAL "\n\n");

    printf("REGISTERS:\n" YELLOW "A" NORMAL ":\t\t\t" BLUE "%d (%08X)" NORMAL "\n" YELLOW "B" NORMAL
           ":\t\t\t" BLUE "%d (%08X)" NORMAL "\n" YELLOW "Program Counter" NORMAL ":\t" BLUE
           "%d (%08X)" NORMAL "\n" YELLOW "Stack Pointer" NORMAL ":\t\t" BLUE "%d (%08X)" NORMAL
           "\n\n",
           registers->A, registers->A, registers->B, registers->B, registers->Program_Counter,
           registers->Program_Counter, registers->Stack_Pointer, registers->Stack_Pointer);

    printf("MEMORY:\n" BLUE "");
    R_Limit = (R_Limit == -1 ? memory->instruction_memory_size : R_Limit + 1);
    for (i = L_limit; i < R_Limit;)
    {
        int till = i + 8;
        for (; i < R_Limit && i < till; ++i)
            printf("%08X ", memory->raw[i]);
        printf("\n");
    }
    printf("" NORMAL "\n");
}

void print_trace(REGISTERS *registers)
{
    printf(YELLOW "A" NORMAL ":\t\t\t" BLUE "%d (%08X)" NORMAL "\n" YELLOW "B" NORMAL ":\t\t\t" BLUE
                  "%d (%08X)" NORMAL "\n" YELLOW "Program Counter" NORMAL ":\t" BLUE
                  "%d (%08X)" NORMAL "\n" YELLOW "Stack Pointer" NORMAL ":\t\t" BLUE
                  "%d (%08X)" NORMAL "\n\n",
           registers->A, registers->A, registers->B, registers->B, registers->Program_Counter,
           registers->Program_Counter, registers->Stack_Pointer, registers->Stack_Pointer);
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
    else if (strcmp(arg, "-read") == 0)
        opt->read_flag = true;
    else if (strcmp(arg, "-write") == 0)
        opt->write_flag = true;
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

void load_file_to_memory(OPTIONS *opt, MEMORY *memory)
{
    FILE *fp = fopen(opt->obj_filename, "rb");
    int file_size;

    if (fp == NULL)
    {
        fprintf(stderr, "[ERROR]: FATAL ERROR");
        exit(-1);
    }

    /* Move the file pointer to the end of the file */
    fseek(fp, 0, SEEK_END);
    /* Store the offset of file pointer from start (i.e. the file size) */
    file_size = ftell(fp);
    /* Bring the file pointer back to start */
    fseek(fp, 0, SEEK_SET);

    if (file_size % 4 != 0)
    {
        fprintf(stderr, "[ERROR]: File not valid object file, bytes: %d", file_size);
        exit(-1);
    }

    /* This should load the object file dump into the memory array */
    fread(memory->raw, 4, file_size / 4, fp);

    fclose(fp);

    memory->instruction_memory_size = file_size / 4;
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

void execute(OPTIONS *opt, REGISTERS *registers, MEMORY *memory)
{
    bool HALT = false;
    int cmd_after_lines = 0;
    while (!HALT)
    {
        unsigned int instruction;
        int oper, operand;

        if (opt->trace)
            print_trace(registers);

        if (cmd_after_lines == 0)
        {
            bool exit_prompt = false;
            while (!exit_prompt)
            {
                char command[100];

                printf("> ");
                fgets(command, 100, stdin);

                /* NULL Termination */
                command[strlen(command) - 1] = '\0';

                trim_whitespace(command);

                if (strcmp(command, "run") == 0)
                    cmd_after_lines = 1, exit_prompt = true;
                else if (strncmp(command, "run ", 4) == 0)
                {
                    if (!is_valid_number(command + 4))
                        fprintf(stderr, RED "INVALID COMMAND" NORMAL "\n");
                    else
                        cmd_after_lines = strtol(command + 4, NULL, 0), exit_prompt = true;
                }
                else if (strcmp(command, "dump") == 0)
                    memory_dump(registers, memory, 0, -1);
                else if (strncmp(command, "dump ", 5) == 0)
                {
                    char *first = strtok(command + 5, " "), *second = strtok(NULL, " ");
                    int a = strtol(first, NULL, 0), b = strtol(second, NULL, 0);
                    if (!is_valid_number(first))
                    {
                        fprintf(stderr, RED "INVALID COMMAND" NORMAL "\n");
                        continue;
                    }

                    if (second == NULL)
                    {
                        memory_dump(registers, memory, strtol(first, NULL, 0), 40);
                        continue;
                    }

                    if (!is_valid_number(second) || a > b)
                    {
                        fprintf(stderr, RED "INVALID COMMAND" NORMAL "\n");
                        continue;
                    }

                    memory_dump(registers, memory, a, b);
                }
                else if (strcmp(command, "exec") == 0)
                    cmd_after_lines = -1, exit_prompt = true;
                else if (strcmp(command, "exit") == 0)
                    HALT = true, exit_prompt = true;
                else
                    fprintf(stderr, RED "INVALID COMMAND" NORMAL "\n");
            }

            if (HALT)
                break;
        }

        if (cmd_after_lines > 0)
            cmd_after_lines--;

        instruction = memory->raw[registers->Program_Counter];

        oper = (instruction % (1 << 8));
        operand = (int)instruction >> 8;

        /* Moving PC ahead, before parsing the instruction */
        registers->Program_Counter++;

        /* INVALID INSTRUCTION CHECK */
        if (oper < 0 || VALID_INSTRUCTION_MNEMONIC_TYPES < oper)
        {
            fprintf(stderr, RED "[ERROR]:" NORMAL " Invalid magic number found: %08X", instruction);
            exit(-1);
        }

        printf("MNEMONIC: " BLUE "%s", mnemonics[oper].OPER);
        if (mnemonics[oper].has_operand)
            printf(" %d", operand);
        printf(NORMAL "\n");

        switch (oper)
        {
        case 0: {
            /**
             * MNEMONIC     OPCODE      Operand
             * ldc          0           Value
             * @brief:
             *      B := A
             *      A := value
             */
            registers->B = registers->A;
            registers->A = operand;
            break;
        }
        case 1: {
            /**
             * MNEMONIC     OPCODE      Operand
             * adc          1           Value
             * @brief:
             *      A := A + value
             */
            registers->A += operand;
            break;
        }
        case 2: {
            /**
             * MNEMONIC     OPCODE      Operand
             * ldl          2           Offset
             * @brief:
             *      B := A
             *      A := memory[SP + offset]
             */
            if (opt->read_flag)
                printf(PURPLE "[READ]:" NORMAL " Memory address " BLUE "%08X" NORMAL " is being "
                              "read.\n",
                       registers->Stack_Pointer + operand);

            registers->B = registers->A;
            if (registers->Stack_Pointer + operand >= MEMORY_SIZE)
            {
                fprintf(stderr, RED "[ERROR]: " NORMAL "Tried to access out of range memory\n");
                exit(-1);
            }
            registers->A = memory->raw[registers->Stack_Pointer + operand];
            break;
        }
        case 3: {
            /**
             * MNEMONIC     OPCODE      Operand
             * stl          3           Offset
             * @brief:
             *      memory[SP + offset] := A
             *      A := B
             */
            if (opt->write_flag)
                printf(PURPLE "[WRITE]:" NORMAL " Memory address " BLUE "%08X" NORMAL " is "
                              "being written over.\n",
                       registers->Stack_Pointer + operand);

            memory->raw[registers->Stack_Pointer + operand] = registers->A;
            if (registers->Stack_Pointer + operand >= MEMORY_SIZE)
            {
                fprintf(stderr, RED "[ERROR]: " NORMAL "Tried to access out of range memory\n");
                exit(-1);
            }
            registers->A = registers->B;
            break;
        }
        case 4: {
            /**
             * MNEMONIC     OPCODE      Operand
             * ldnl         4           offset
             * @brief:
             *      A := memory[A + offset]
             */
            if (opt->read_flag)
                printf(PURPLE "[READ]:" NORMAL " Memory address " BLUE "%08X" NORMAL " is being "
                              "read.\n",
                       registers->A + operand);

            if (registers->Stack_Pointer + operand >= MEMORY_SIZE)
            {
                fprintf(stderr, RED "[ERROR]: " NORMAL "Tried to access out of range memory\n");
                exit(-1);
            }
            registers->A = memory->raw[registers->A + operand];
            break;
        }
        case 5: {
            /**
             * MNEMONIC     OPCODE      Operand
             * stnl         5           offset
             * @brief:
             *      memory[A + offset] := B
             */
            if (opt->write_flag)
                printf(PURPLE "[WRITE]:" NORMAL " Memory address " BLUE "%08X" NORMAL " is "
                              "being written over.\n",
                       registers->A + operand);

            if (registers->Stack_Pointer + operand >= MEMORY_SIZE)
            {
                fprintf(stderr, RED "[ERROR]: " NORMAL "Tried to access out of range memory\n");
                exit(-1);
            }
            memory->raw[registers->A + operand] = registers->B;
            break;
        }
        case 6: {
            /**
             * MNEMONIC     OPCODE      Operand
             * add          6           NONE
             * @brief:
             *      A := B + A
             */
            registers->A = (registers->B + registers->A);
            break;
        }
        case 7: {
            /**
             * MNEMONIC     OPCODE      Operand
             * sub          7           NONE
             * @brief:
             *      A := B - A
             */
            registers->A = (registers->B - registers->A);
            break;
        }
        case 8: {
            /**
             * MNEMONIC     OPCODE      Operand
             * shl          7           NONE
             * @brief:
             *      A := B << A
             */
            registers->A = (registers->B << registers->A);
            break;
        }
        case 9: {
            /**
             * MNEMONIC     OPCODE      Operand
             * shr          8           NONE
             * @brief:
             *      A := B >> A
             */
            registers->A = (registers->B >> registers->A);
            break;
        }
        case 10: {
            /**
             * MNEMONIC     OPCODE      Operand
             * adj          9           Value
             * @brief:
             *      SP := SP + value
             */
            registers->Stack_Pointer += operand;
            break;
        }
        case 11: {
            /**
             * MNEMONIC     OPCODE      Operand
             * a2sp         10          NONE
             * @brief:
             *      SP := A
             *      A := B
             */
            registers->Stack_Pointer = registers->A;
            registers->A = registers->B;
            break;
        }
        case 12: {
            /**
             * MNEMONIC     OPCODE      Operand
             * sp2a         11          NONE
             * @brief:
             *      B := A
             *      A := SP
             */
            registers->B = registers->A;
            registers->A = registers->Stack_Pointer;
            break;
        }
        case 13: {
            /**
             * MNEMONIC     OPCODE      Operand
             * call         12          Offset
             * @brief:
             *      B := A
             *      A := PC
             *      PC := PC + offset
             */
            registers->B = registers->A;
            registers->A = registers->Program_Counter;
            registers->Program_Counter += operand;
            break;
        }
        case 14: {
            /**
             * MNEMONIC     OPCODE      Operand
             * return       14          NONE
             * @brief:
             *      PC := A
             *      A := B
             */
            registers->Program_Counter = registers->A;
            registers->A = registers->B;
            break;
        }
        case 15: {
            /**
             * MNEMONIC     OPCODE      Operand
             * brz          15          Offset
             * @brief:
             *      if A == 0 then
             *          PC := PC + offset
             */
            if ((int)registers->A == 0)
                registers->Program_Counter += operand;
            break;
        }
        case 16: {
            /**
             * MNEMONIC     OPCODE      Operand
             * brlz         16          Offset
             * @brief:
             *      if A < 0 then
             *          PC := PC + offset
             */
            if ((int)registers->A < 0)
                registers->Program_Counter += operand;
            break;
        }
        case 17: {
            /**
             * MNEMONIC     OPCODE      Operand
             * br           17          Offset
             * @brief:
             *      PC := PC + offset
             */
            registers->Program_Counter += operand;
            break;
        }
        case 18: {
            /**
             * MNEMONIC     OPCODE      Operand
             * HALT         18          NONE
             * @brief:
             *      Halt := true
             */
            HALT = true;
            printf("\n" GREEN "Program Halted!" NORMAL "\n\n");
            break;
        }
        default: {
            fprintf(stderr, "[ERROR]: Invalid magic number found: %08X", instruction);
            exit(-1);
        }
        }
    }
}

void destruct_and_exit(OPTIONS *opt, int status_code)
{
    free(opt->obj_filename);
    exit(status_code);
}

int main(int argc, char **argv)
{
    OPTIONS opt = {false, false, false, false, false, false, NULL};

    /* Memory and Register Initialization */
    static REGISTERS Registers = {0, 0, 0, 0};
    static MEMORY Memory = {{0}, 0};

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
        else if (strcmp(argv[1], "-isa") == 0)
        {
            print_isa();
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

    if (opt.isa)
        print_isa();

    load_file_to_memory(&opt, &Memory);

    if (opt.mem_dump_before)
        memory_dump(&Registers, &Memory, 0, -1);

    execute(&opt, &Registers, &Memory);

    if (opt.mem_dump_after)
        memory_dump(&Registers, &Memory, 0, -1);

    destruct_and_exit(&opt, 0);
    return 0;
}
