#ifndef PARSER
#define PARSER

#define WORD_SIZE 16

#define COMMAND_BYTES 2

#define EOP "\0\0"
#define OPERATION "\0\5"
#define O_CREATE_VAR "\0\10"

#define ADD_OPERATION "\0\1"

struct token_list;

/*
16 bytes
2 4
CreateVar Type

2 2 4 4 4?
Operation Add pointer 1 pointer 2
Minus
Multiply
Divide
Potency
Equals

2 4 4 4 2?
Function Line Scope_Start, Scope_End

32
END OF PROGRAM = 0
*/

void *parse_tokens(struct token_list *token_list);

#endif