#ifndef AURORA_PARSER
#define AURORA_PARSER

#include <stdint.h>

#define WORD_SIZE 16

#define ADDRESS_BYTES 4
#define TYPE_BYTES 2
#define COMMAND_BYTES 2

#define C_EOP "\x00\x00"

/// 2 4 2 ??
/// Assign pointer type value
#define C_ASSIGN "\x00\x04"
#define C_OPERATION "\x00\x05"

#define C_CREATE_VAR "\x00\x10"
#define C_PRINT "\x00\x15"

/// 2 1 4
/// Jump isRef address
#define C_JUMP "\x00\x20"

/// 2 4
/// EnterBlock StackAddr
#define C_SETBLOCK "\x00\x21"

/// 2
/// BackBlock
#define C_BACKBLOCK "\x00\x22"

#define OPERATION_BYTES 2
#define O_ADD "\x00\x01"

struct token_list;

/*
16 bytes

2 4
EnterBlock StackAddr

2 4 4 2
Assign pointer value type

2 4 2
CreateVar Position Type

2 4 4
Print pointer Type

2 2 4 4 4?
Operation Add pointer 1 pointer 2
Minus
Multiply
Divide
Potency
Equals

2 4
RedefineStart

2 4
Function Calling
Stack
    0 - 4 - Stack start address
    4 - 8 - Return address
    8 - ?? - Arguments

2 4 4 4 2?
Function Line Scope_Start, Scope_End

32
END OF PROGRAM = 0
*/

char *parse_tokens(struct token_list *token_list, int32_t memory_offset);

#endif