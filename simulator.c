/*
  Author: Daniel Kopta and ??
  Spring 2018
  CS 4400, University of Utah

  * A simple x86-like processor simulator.
  * Read in a binary file that encodes instructions to execute.
  * Simulate a processor by executing instructions one at a time and appropriately 
  * updating register and memory contents.

  * Some code and pseudo code has been provided as a starting point.

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "instruction.h"

// Forward declarations for helper functions
unsigned int get_file_size(int file_descriptor);
unsigned int* load_file(int file_descriptor, unsigned int size);
instruction_t* decode_instructions(unsigned int* bytes, unsigned int num_instructions);
unsigned int execute_instruction(unsigned int program_counter, instruction_t* instructions, 
				 int* registers, unsigned char* memory);
void print_instructions(instruction_t* instructions, unsigned int num_instructions);
void error_exit(const char* message);

// 17 registers
#define NUM_REGS 17
// 1024-byte stack
#define STACK_SIZE 1024

int main(int argc, char** argv)
{
  // Make sure we have enough arguments
  if(argc < 2)
    error_exit("must provide an argument specifying a binary file to execute");

  // Open the binary file
  int file_descriptor = open(argv[1], O_RDONLY);
  if (file_descriptor == -1) 
    error_exit("unable to open input file");

  // Get the size of the file
  unsigned int file_size = get_file_size(file_descriptor);
  // Make sure the file size is a multiple of 4 bytes
  // since machine code instructions are 4 bytes each
  if(file_size % 4 != 0)
    error_exit("invalid input file");

  // Load the file into memory
  // We use an unsigned int array to represent the raw bytes
  // We could use any 4-byte integer type
  unsigned int* instruction_bytes = load_file(file_descriptor, file_size);
  close(file_descriptor);

  unsigned int num_instructions = file_size / 4;


  /****************************************/
  /**** Begin code to modify/implement ****/
  /****************************************/

  // Allocate and decode instructions (left for you to fill in)
  instruction_t* instructions = decode_instructions(instruction_bytes, num_instructions);

  // Optionally print the decoded instructions for debugging
  // Will not work until you implement decode_instructions
  // Do not call this function in your handed in final version
  //print_instructions(instructions, num_instructions);


// Once you have completed part 1 (decoding instructions), uncomment the below block


  // Allocate and initialize registers
  int* registers = (int*)malloc(sizeof(int) * NUM_REGS);
  // TODO: initialize register values
  for(int i = 0; i < NUM_REGS; i++)
  {
      registers[i] = 0;
  }
  registers[6] = STACK_SIZE;
  // Stack memory is byte-addressed, so it must be a 1-byte type
  // TODO allocate the stack memory. Do not assign to NULL.
  unsigned char* memory = (unsigned char*) malloc(sizeof(char) * STACK_SIZE);
  for(int i = 0; i < STACK_SIZE; i++)
  {
    memory[i] = 0;
  }

  // Run the simulation
  unsigned int program_counter = 0;

  // program_counter is a byte address, so we must multiply num_instructions by 4 to get the address past the last instruction
  while(program_counter != num_instructions * 4)
  {
    program_counter = execute_instruction(program_counter, instructions, registers, memory);
  }
  return 0;
}



/*
 * Decodes the array of raw instruction bytes into an array of instruction_t
 * Each raw instruction is encoded as a 4-byte unsigned int
*/
instruction_t* decode_instructions(unsigned int* bytes, unsigned int num_instructions)
{
  instruction_t* retval = (instruction_t*) malloc(sizeof(instruction_t) * num_instructions);


  int i;
  for(i = 0; i < num_instructions; i++){

    retval[i].opcode = (bytes[i]>>27) & 0x1f; 

    retval[i].first_register = (bytes[i]>>22) & 0x1f;

    retval[i].second_register = (bytes[i]>>17) & 0x1f;

    retval[i].immediate = ((int16_t) bytes[i]) & 0xffff;
  }
    
  return retval;
}


/*
 * Executes a single instruction and returns the next program counter
*/
unsigned int execute_instruction(unsigned int program_counter, instruction_t* instructions, int* registers, unsigned char* memory)
{
  // program_counter is a byte address, but instructions are 4 bytes each
  // divide by 4 to get the index into the instructions array
  instruction_t instr = instructions[program_counter / 4];
  long longResult;

  switch(instr.opcode)
  {
  case subl:
    registers[instr.first_register] = registers[instr.first_register] - instr.immediate;
    break;
  case addl_reg_reg:
    registers[instr.second_register] = registers[instr.first_register] + registers[instr.second_register];
    break;
  case addl_imm_reg:
    registers[instr.first_register] = registers[instr.first_register] + instr.immediate;
    break;
  case imull:
    registers[instr.second_register] = registers[instr.first_register] * registers[instr.second_register];
    break;
  case shrl:
    registers[instr.first_register] = ((unsigned int) registers[instr.first_register]) >> 1;
    break;
  case movl_reg_reg:
    registers[instr.second_register] = registers[instr.first_register];
    break;
  case movl_deref_reg:
    //reg2 = memory[reg1 + imm] 
    registers[instr.second_register] = *((unsigned int*) (memory + registers[instr.first_register] + instr.immediate));//memory[instr.first_register + instr.immediate];
    break;
  case movl_reg_deref:
  //memory[reg2 + imm] = reg1
    *((unsigned int*) (memory + registers[instr.second_register] + instr.immediate)) = registers[instr.first_register];
    break;
  case movl_imm_reg:
    registers[instr.first_register] = instr.immediate;
    //registers[instr.first_register] = sign_extend(instr.immediate);
    break;
  case cmpl:
  //if reg1 > reg 2 then CF
    longResult = (((long) registers[instr.second_register]) - ((long)registers[instr.first_register]));
    registers[16] = 0;

    if((unsigned) registers[instr.first_register] > (unsigned) registers[instr.second_register])
    {
      //CF is true
      registers[16] += 0x1;
    }
    if (longResult == 0)
    {
      //ZF is true
      registers[16] += 0x40;
    }
    if ((longResult & 0x80000000))
    {
      //SF is true
      registers[16] += 0x80;
    }
    if (longResult > 2147483647 || longResult < -2147483648)
    {
      //OF is true
      registers[16] += 0x800;
    }
    break;
  case je:
    if((registers[16] & (1 << 6)))
    {
      program_counter += instr.immediate;
    }
    break;
  case jl:
  //SF xor OF 7 or 11
    if((registers[16] & (1 << 7)) ^ (registers[16] & (1 << 11)))
    {
      program_counter += instr.immediate;
    }
    break;
  case jle:
  //(SF xor OF) or ZF
    if(((registers[16] & (1 << 7)) ^ ((registers[16] & (1 << 11)))) || (registers[16] & (1 << 6)))
    {
      program_counter += instr.immediate;
    }
    break;
  case jge:
  //not (SF xor OF)
    if(!((registers[16] & (1 << 7)) ^ (registers[16] & (1 << 11))))
    {
      program_counter += instr.immediate;
    }
    break;
  case jbe:
  //CF or ZF
    if((registers[16] & (1 << 0)) || (registers[16] & (1 << 6)) )
    {
      program_counter += instr.immediate;
    }
    break;
  case jmp:
    program_counter += instr.immediate;
    break;
  case call:
    /*%esp = %esp - 4
    memory[%esp] = program_counter + 4
    jump to target (see below)*/
    registers[6] -= 4;
    *((unsigned int*) memory + registers[6]) = (program_counter + 4);
    program_counter += instr.immediate;
    break;
  case ret:
    /*
    if 
      %esp == 1024, exit simulation
    else
      program_counter = memory[%esp]
    %esp = %esp + 4
    */
    if(registers[6] == STACK_SIZE)
    {
      exit(0);
    }
    else
    {
      program_counter = *((unsigned int*) memory + registers[6]);
      registers[6] += 4;
      return program_counter;
    }
    break;
  case pushl:
    /*
      %esp = %esp - 4
      memory[%esp] = reg1
    */
    registers[6] -= 4;
    *((unsigned int*) (memory + registers[6])) = registers[instr.first_register];
    break;
  case popl:
    /*
    reg1 = memory[%esp]
    %esp = %esp + 4
    */
    registers[instr.first_register] = *((unsigned int*) (memory + registers[6]));
    registers[6] += 4;
    break;
  case printr:
    printf("%d (0x%x)\n", registers[instr.first_register], registers[instr.first_register]);
    break;
  case readr:
    scanf("%d", &(registers[instr.first_register]));
    break;

  }

  // TODO: Don't always return program_counter + 4
  //       Some instructions will jump elsewhere

  // program_counter + 4 represents the subsequent instruction
  return program_counter + 4;
}


/*********************************************/
/**** Begin helper methods. Do not modify ****/
/*********************************************/

/*
 * Returns the file size in bytes of the file referred to by the given descriptor
*/
unsigned int get_file_size(int file_descriptor)
{
  struct stat file_stat;
  fstat(file_descriptor, &file_stat);
  return file_stat.st_size;
}

/*
 * Loads the raw bytes of a file into an array of 4-byte units
*/
unsigned int* load_file(int file_descriptor, unsigned int size)
{
  unsigned int* raw_instruction_bytes = (unsigned int*)malloc(size);
  if(raw_instruction_bytes == NULL)
    error_exit("unable to allocate memory for instruction bytes (something went really wrong)");

  int num_read = read(file_descriptor, raw_instruction_bytes, size);

  if(num_read != size)
    error_exit("unable to read file (something went really wrong)");

  return raw_instruction_bytes;
}

/*
 * Prints the opcode, register IDs, and immediate of every instruction, 
 * assuming they have been decoded into the instructions array
*/
void print_instructions(instruction_t* instructions, unsigned int num_instructions)
{
  printf("instructions: \n");
  unsigned int i;
  for(i = 0; i < num_instructions; i++)
  {
    printf("op: %d, reg1: %d, reg2: %d, imm: %d\n", 
	   instructions[i].opcode,
	   instructions[i].first_register,
	   instructions[i].second_register,
	   instructions[i].immediate);
  }
  printf("--------------\n");
}


/*
 * Prints an error and then exits the program with status 1
*/
void error_exit(const char* message)
{
  printf("Error: %s\n", message);
  exit(1);
}
