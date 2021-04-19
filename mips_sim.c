// COMP1521 20T3 Assignment 1: mips_sim -- a MIPS simulator
// starting point code v0.1 - 13/10/20

//z5165630

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_LENGTH 256
#define INSTRUCTIONS_GROW 64

typedef struct instruction_components {
    //No immediate vals, 5 chonks
    //6 chonks of bit pattern
    uint32_t padding;
    uint32_t op1;
    uint32_t op2;
    uint32_t dest;
    uint32_t id;

} instruction_components_t;

typedef struct im_instruction_components {
    uint32_t id;
    uint32_t op1;
    uint32_t op2;
    int16_t im;

} im_instruction_components_t;


//My #defines
#define SYSCALL 0b1100
#define OP1_SHIFT 21
#define OP2_SHIFT 16
#define PADDING_SHIFT 26
#define FIVE_BIT_MASK 0b11111
#define SIX_BIT_MASK 0b111111
#define ELEVEN_BIT_MASK 0b11111111111
#define SIXTEEN_BIT_MASK 0b1111111111111111


void execute_instructions(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode);
char *process_arguments(int argc, char *argv[], int *trace_mode);
uint32_t *read_instructions(char *filename, int *n_instructions_p);
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions);

//My prototypes

instruction_components_t five_piece(uint32_t inst);
im_instruction_components_t four_piece(uint32_t inst);
void add_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag);
void slt_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag);
void print_helper(uint32_t dest, uint32_t op1, uint32_t op2,int finalVal);
void beq_syscall(uint32_t hexInstruction, int *instructionCount, uint32_t *registers, int tmFlag);
void addi_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag);
void bne_syscall(uint32_t hexInstruction, int *instructionCount, uint32_t *registers, int tmFlag);
void ori_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag);
void syscallChecker(uint32_t *registers, int tmFlag);
void lui_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag);
void mul_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag);
void sub_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registersint, int tmFlag);


// YOU SHOULD NOT NEED TO CHANGE MAIN
int main(int argc, char *argv[]) {
    int trace_mode;
    char *filename = process_arguments(argc, argv, &trace_mode);

    int n_instructions;
    uint32_t *instructions = read_instructions(filename, &n_instructions);

    execute_instructions(n_instructions, instructions, trace_mode);

    free(instructions);
    return 0;
}


// simulate execution of  instruction codes in  instructions array
// output from syscall instruction & any error messages are printed
//
// if trace_mode != 0:
//     information is printed about each instruction as it executed
//
// execution stops if it reaches the end of the array

void execute_instructions(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode) {

    //Store registers - unsigned 32bit int
    uint32_t *registers = (uint32_t *)calloc(32, sizeof(uint32_t));
    
    int pc = 0;

    while (pc < n_instructions && pc >= 0) {
        instruction_components_t instructionParts = five_piece(instructions[pc]);

            if(trace_mode) {
               printf("%d: 0x%08X ", pc, instructions[pc]); 
            }
            
            //If statement to call appropriate function based on bit pattern
            if(instructions[pc] == 0xc) {
                syscallChecker(registers, trace_mode);
            } else if(instructionParts.padding == 0 && instructionParts.id == 0b100000){
                //ori_syscall(instArray[pc],pc,registers,1);
                add_syscall(instructions[pc], pc, registers, trace_mode);
            } else if (instructionParts.padding == 0 && instructionParts.id == 0b100010) {
                sub_syscall(instructions[pc], pc, registers, trace_mode);
            } else if (instructionParts.padding == 0 && instructionParts.id == 0b101010) {
                slt_syscall(instructions[pc], pc, registers, trace_mode);
            } else if (instructionParts.padding == 0b011100 && instructionParts.id == 0b000010) {
                mul_syscall(instructions[pc], pc, registers, trace_mode);
            } else if (instructionParts.padding == 0b000100) {
                beq_syscall(instructions[pc], &pc, registers, trace_mode);
            } else if (instructionParts.padding == 0b000101) {
                bne_syscall(instructions[pc], &pc, registers, trace_mode);
            } else if (instructionParts.padding == 0b001000) {
                addi_syscall(instructions[pc], pc, registers, trace_mode);
            } else if (instructionParts.padding == 0b001101) {
                ori_syscall(instructions[pc], pc, registers, trace_mode);
            } else if (instructionParts.padding == 0b001111 && instructionParts.op1 == 0) {
                lui_syscall(instructions[pc], pc, registers, trace_mode);
            } else {
                printf("invalid instruction code\n");
                exit(0);
            }

            //$v0 should never be changed, reset after every instruction
            registers[0] = 0;
            if(pc > n_instructions) {
                //branch beyond last instruction
                printf("Illegal branch to address after instructions: PC = %d\n",pc+1);
                exit(0);
            } else if (pc < 0) {
                //branch prior to first instruction
                printf("Illegal branch to address before instructions: PC = %d\n",pc+1);
                exit(0);
            } else {
                pc++;
            }

        }

}



// My functions

//Populate fields of 5 part struct (for non immediate functions)
instruction_components_t five_piece(uint32_t inst) {    
    instruction_components_t components;
    //uint32_t mask = 0b111111;
    //32-6 = 26
    components.padding =  SIX_BIT_MASK & (inst >> PADDING_SHIFT);
    //mask = 0b11111;
    //32-6-5 = 21
    components.op1 = FIVE_BIT_MASK &(inst >> OP1_SHIFT);
    //32-6-5-5 = 16
    components.op2 = FIVE_BIT_MASK  &(inst >> OP2_SHIFT);
    //32-6-5-5-5 = 16
    components.dest = FIVE_BIT_MASK  &(inst >> 11);
    //mask = 0b11111111111;
    components.id = ELEVEN_BIT_MASK  & inst;
    return components;

}

//Populate fields of 4 part struct (for immediate functions)
im_instruction_components_t four_piece(uint32_t inst) {    
    im_instruction_components_t im_components;
    //uint32_t mask = 0b111111;
    //32-6 = 26
    im_components.id = SIX_BIT_MASK &(inst >> 26);
    //mask = 0b11111;
    //32-6-5 = 21
    im_components.op1 = FIVE_BIT_MASK &(inst >> OP1_SHIFT); 
    //32-6-5-5 = 16
    im_components.op2 = FIVE_BIT_MASK &(inst >> OP2_SHIFT);
    //32-6-5-5-5 = 16
    //mask = 0b1111111111111111;
    im_components.im = SIXTEEN_BIT_MASK & inst;
    return im_components;  

}


void beq_syscall(uint32_t hexInstruction, int *instructionCount, uint32_t *registers, int tmFlag) {    
    im_instruction_components_t hexPieces = four_piece(hexInstruction);
    //to determine print statement
    int branch_flag = 0;
    if(registers[hexPieces.op1] == registers[hexPieces.op2]) {
        //derefernce
        *instructionCount += hexPieces.im;
        //Remove incrementing, this is done in main
        *instructionCount -= 1;
        branch_flag = 1;
    }
    //if in tracemode,
    if(tmFlag) {
        printf("beq  ");   
        printf("$%d, ", hexPieces.op1);
        printf("$%d, ", hexPieces.op2);
        printf("%d\n", hexPieces.im);
        if(branch_flag) {
            printf(">>> branch taken to PC = %d\n", (*instructionCount)+1);        
        } else {
            printf(">>> branch not taken\n");
        }
    }    
}


void bne_syscall(uint32_t hexInstruction, int *instructionCount, uint32_t *registers, int tmFlag) {    
    im_instruction_components_t hexPieces = four_piece(hexInstruction);
    //to detiremine print statement
    int branch_flag = 0;
    if(registers[hexPieces.op1] != registers[hexPieces.op2]) {
        //derefernce
        //shift im places in array of instructions
        *instructionCount += hexPieces.im;
        //Remove incrementing, this is done in main
        *instructionCount -= 1;
        branch_flag = 1;
    }
    //if in tracemode,
    if(tmFlag) {
        printf("bne  ");   
        printf("$%d, ", hexPieces.op1);
        printf("$%d, ", hexPieces.op2);
        printf("%d\n", hexPieces.im);
        if(branch_flag) {
            printf(">>> branch taken to PC = %d\n", (*instructionCount)+1);
            
        } else {
            printf(">>> branch not taken\n");
        }
    }    
}


void addi_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {    
    im_instruction_components_t hexPieces = four_piece(hexInstruction);

    int result = registers[hexPieces.op1] + hexPieces.im;
    registers[hexPieces.op2] = result;
    //if in tracemode,
    if(tmFlag) {
        printf("addi ");   
        printf("$%d, ", hexPieces.op2);
        printf("$%d, ", hexPieces.op1);
        printf("%d\n", hexPieces.im);
        printf(">>> $%d = %d\n", hexPieces.op2,result);
    }

}

void ori_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {    
    im_instruction_components_t hexPieces = four_piece(hexInstruction);

    int result = registers[hexPieces.op1] | hexPieces.im;
    registers[hexPieces.op2] = result;

    //if in tracemode,
    if(tmFlag) {
        printf("ori  ");   
        printf("$%d, ", hexPieces.op2);
        printf("$%d, ", hexPieces.op1);
        printf("%d\n", hexPieces.im);
        printf(">>> $%d = %d\n", hexPieces.op2,result);
    }
}


void mul_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {    
    instruction_components_t hexPieces = five_piece(hexInstruction);
    uint32_t result = (registers[hexPieces.op1]) * (registers[hexPieces.op2]);
    //store result in destination register
    registers[hexPieces.dest] = result;

    //if in tracemode,
    if(tmFlag) {
        printf("mul  ");
        print_helper(hexPieces.dest, hexPieces.op1, hexPieces.op2, result);
    }
    
}

void add_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {    
    instruction_components_t hexPieces = five_piece(hexInstruction);

    //extract value that is there
    uint32_t result = registers[hexPieces.op1] + registers[hexPieces.op2];
    registers[hexPieces.dest] = result;

    //if in tracemode,
    if(tmFlag) {
        printf("add  ");
        printf("$%d, ", hexPieces.dest);
        printf("$%d, ", hexPieces.op1);
        printf("$%d\n", hexPieces.op2);
        printf(">>> $%d = %d\n", hexPieces.dest,result);
    }
    

}

void slt_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {
    instruction_components_t hexPieces = five_piece(hexInstruction);   
    //if s < t set dest to 1
    if(registers[hexPieces.op1] < registers[hexPieces.op2]) {
        registers[hexPieces.dest] = 1;
    //0 otherwise
    } else {
        registers[hexPieces.dest] = 0;
    }    
    //if in tracemode,
    if(tmFlag) {
        printf("slt  ");
        print_helper(hexPieces.dest,hexPieces.op1,hexPieces.op2,registers[hexPieces.dest]);
    } 
    
}

void print_helper(uint32_t dest, uint32_t op1, uint32_t op2, int finalVal) {
    printf("$%d, ", dest);
    printf("$%d, ", op1);
    printf("$%d\n", op2);
    printf(">>> $%d = %d\n", dest,finalVal);
}

void lui_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {    
    im_instruction_components_t hexPieces = four_piece(hexInstruction);  
    int result = hexPieces.im << 16;
    //store shifted value in destination register
    registers[hexPieces.op2] = result;

    //if in tracemode
    if(tmFlag) {
        printf("lui  ");   
        printf("$%d, ", hexPieces.op2);
        printf("%d\n", hexPieces.im);
        printf(">>> $%d = %d\n", hexPieces.op2,result);
    }


}

void sub_syscall(uint32_t hexInstruction, int instructionCount, uint32_t *registers, int tmFlag) {
    instruction_components_t hexPieces = five_piece(hexInstruction);

    uint32_t result = registers[hexPieces.op1] - registers[hexPieces.op2];
    registers[hexPieces.dest] = result;

    //if in tracemode
    if(tmFlag) {
        printf("sub ");
        print_helper(hexPieces.dest, hexPieces.op1, hexPieces.op2, result);
    }


}

void syscallChecker(uint32_t *registers, int tmFlag) {
    //$a0 is at index 4
    uint32_t a0Ref = registers[4];
    //$v0 is at index 2
    uint32_t v0Ref = registers[2];
    if(tmFlag) {
        printf("syscall\n");
        printf(">>> syscall %d\n",v0Ref);
    }
   
    if(v0Ref == 1) {
         if(tmFlag) {
             printf("<<< %d\n", a0Ref);
         } else {
             printf("%d", a0Ref);
         }
         
    } else if (v0Ref == 10) {
         exit(0);

    } else if (v0Ref == 11) {
        if(tmFlag) {
             printf("<<< %c\n", a0Ref & 0xff);
         } else {
             printf("%c", a0Ref & 0xff);
         }

    } else {
        printf("Unknown system call: %d\n",v0Ref);
        exit(0);
        
    }

}



// YOU DO NOT NEED TO CHANGE CODE BELOW HERE


// check_arguments is given command-line arguments
// it sets *trace_mode to 0 if -r is specified
//          *trace_mode is set to 1 otherwise
// the filename specified in command-line arguments is returned

char *process_arguments(int argc, char *argv[], int *trace_mode) {
    if (
        argc < 2 ||
        argc > 3 ||
        (argc == 2 && strcmp(argv[1], "-r") == 0) ||
        (argc == 3 && strcmp(argv[1], "-r") != 0)) {
        fprintf(stderr, "Usage: %s [-r] <file>\n", argv[0]);
        exit(1);
    }
    *trace_mode = (argc == 2);
    return argv[argc - 1];
}


// read hexadecimal numbers from filename one per line
// numbers are return in a malloc'ed array
// *n_instructions is set to size of the array

uint32_t *read_instructions(char *filename, int *n_instructions_p) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: '%s'\n", strerror(errno), filename);
        exit(1);
    }

    uint32_t *instructions = NULL;
    int n_instructions = 0;
    char line[MAX_LINE_LENGTH + 1];
    while (fgets(line, sizeof line, f) != NULL) {

        // grow instructions array in steps of INSTRUCTIONS_GROW elements
        if (n_instructions % INSTRUCTIONS_GROW == 0) {
            instructions = instructions_realloc(instructions, n_instructions + INSTRUCTIONS_GROW);
        }

        char *endptr;
        instructions[n_instructions] = strtol(line, &endptr, 16);
        if (*endptr != '\n' && *endptr != '\r' && *endptr != '\0') {
            fprintf(stderr, "%s:line %d: invalid hexadecimal number: %s",
                    filename, n_instructions + 1, line);
            exit(1);
        }
        n_instructions++;
    }
    fclose(f);
    *n_instructions_p = n_instructions;
    // shrink instructions array to correct size
    instructions = instructions_realloc(instructions, n_instructions);
    return instructions;
}


// instructions_realloc is wrapper for realloc
// it calls realloc to grow/shrink the instructions array
// to the speicfied size
// it exits if realloc fails
// otherwise it returns the new instructions array
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions) {
    instructions = realloc(instructions, n_instructions * sizeof *instructions);
    if (instructions == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    return instructions;
}