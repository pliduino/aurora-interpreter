#include "parser.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "lexer/lexer.h"
#include "parser/function_array.h"
#include "parser/variable_array.h"

#define BUFFER 64

static struct parse_vars
{
    struct token **token_buffer;
    size_t token_buffer_count;
    struct variable_array *variable_array;
    struct function_array *function_array;
    int32_t cur_word;
    bool has_error;
    char *parsed;
};

inline static void next_word(struct parse_vars *parse_vars)
{
    parse_vars->cur_word++;
    if (parse_vars->cur_word % BUFFER == 0)
    {
        parse_vars->parsed =
            realloc(parse_vars->parsed, WORD_SIZE * (parse_vars->cur_word + BUFFER));
    }
}

/**================================================== *
 * ===================  Commands  =================== *
 * ================================================== */

static inline void command_backblock(struct parse_vars *parse_vars)
{
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_BACKBLOCK, COMMAND_BYTES);
    next_word(parse_vars);
}

static inline void command_setblock(struct parse_vars *parse_vars)
{
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_BACKBLOCK, COMMAND_BYTES);
    next_word(parse_vars);
}

static inline void command_print(struct parse_vars *parse_vars, address position,
                                 enum variable_type type)
{
    int16_t two_byte_type = type;

    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_PRINT, COMMAND_BYTES);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES],
              (char *)&position, ADDRESS_BYTES);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES],
              (char *)&two_byte_type, TYPE_BYTES);
    next_word(&parse_vars);
}

static inline void command_create_var(struct parse_vars *parse_vars)
{
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_BACKBLOCK, COMMAND_BYTES);
    next_word(parse_vars);
}

static inline void command_jump(struct parse_vars *parse_vars, enum jump_type jump_type,
                                uint32_t address)
{
    char jump_type_bool = jump_type;

    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_JUMP, COMMAND_BYTES);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES],
              &jump_type_bool, 1);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + 1],
              (char *)&address, ADDRESS_BYTES);
    next_word(parse_vars);
}

static inline void command_assign(struct parse_vars *parse_vars, enum variable_type type,
                                  uint32_t position, char *value_text)
{
    int16_t two_byte_type = (int16_t)type;

    // Assign
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_ASSIGN, COMMAND_BYTES);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES],
              (char *)&position, ADDRESS_BYTES);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES],
              (char *)&two_byte_type, TYPE_BYTES);

    switch (type)
    {
    case BOOL:
        char value_bool;
        if (strcmp(value_text, "true") == 0)
        {
            value_bool = 1;
        }
        else if (strcmp(value_text, "false") == 0)
        {
            value_bool = 0;
        }
        else
        {
            return;
        }
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_bool, get_size_of_type(BOOL));
        break;
    case CHAR:
        char value_char = (char)atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_char, get_size_of_type(CHAR));
        break;
    case I8:
        int8_t value_int8 = (int8_t)atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_int8, get_size_of_type(I8));
        break;
    case I16:
        int16_t value_int16 = (int16_t)atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_int16, get_size_of_type(I16));
        break;
    case I32:
        int32_t value_int32 = atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_int32, get_size_of_type(I32));
        break;
    case I64:
        int64_t value_int64 = atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_int64, get_size_of_type(I64));
        break;
    case U8:
        uint8_t value_uint8 = (uint8_t)atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_uint8, get_size_of_type(U8));
        break;
    case U16:
        uint16_t value_uint16 = (uint16_t)atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_uint16, get_size_of_type(U16));
        break;
    case U32:
        uint32_t value_uint32 = atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_uint32, get_size_of_type(U32));
        break;
    case U64:
        uint64_t value_uint64 = atoi(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_uint64, get_size_of_type(U64));
        break;
    case F32:
        float value_float = (float)atof(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_float, get_size_of_type(F32));
        break;
    case F64:
        double value_double = atof(value_text);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                      ADDRESS_BYTES + TYPE_BYTES],
                  (char *)&value_double, get_size_of_type(F64));
        break;
    case INVALID:
        break;
    default:
        break;
    }

    next_word(parse_vars);
}

/* ===============  End of Commands  ================ */

static inline void print(struct parse_vars *parse_vars)
{
    if (parse_vars->token_buffer_count < 4)
    {
        fprintf(stderr, "%s:%d: Not enough tokens!\n", "file", parse_vars->token_buffer[0]->line);
        parse_vars->has_error = true;
        return;
    }
    if (parse_vars->token_buffer[1]->type != CALLER_START)
    {
        fprintf(stderr, "%s:%d: Caller Start unavailable!\n", "file",
                parse_vars->token_buffer[0]->line);
        parse_vars->has_error = true;
        return;
    }
    if (parse_vars->token_buffer[parse_vars->token_buffer_count - 1]->type != CALLER_END)
    {
        fprintf(stderr, "%s:%d: Caller end missing!\n", "file", parse_vars->token_buffer[0]->line);
        parse_vars->has_error = true;
        return;
    }

    int64_t index_return =
        variable_array_find(parse_vars->variable_array, parse_vars->token_buffer[2]->text);
    if (index_return < 0)
    {
        fprintf(stderr, "%s:%d: Variable %s does not exist to print!\n", "file",
                parse_vars->token_buffer[2]->line, parse_vars->token_buffer[2]->text);
        parse_vars->has_error = true;
        return;
    }
    uint32_t index = (int32_t)index_return;

    command_print(&parse_vars, parse_vars->variable_array->data[index].position,
                  parse_vars->variable_array->data[index].type);
}

static inline void create_var(struct parse_vars *parse_vars)
{
    if (parse_vars->token_buffer_count < 3)
    {
        fprintf(stderr, "%s:%d: Statement has not enough arguments!\n", "filename",
                parse_vars->token_buffer[0]->line);
        parse_vars->has_error = true;
        return;
    }
    if (parse_vars->token_buffer[1]->type != NAME || parse_vars->token_buffer[2]->type != NAME)
    {
        fprintf(stderr, "%s:%d: Statement requires a type and a name!\n", "filename",
                parse_vars->token_buffer[0]->line);
        parse_vars->has_error = true;
        return;
    }
    struct variable variable = {
        .name = parse_vars->token_buffer[2]->text,
        .type = variable_type_from_string(parse_vars->token_buffer[1]->text),
        .position = parse_vars->variable_array->head,
    };
    if (variable.type == INVALID)
    {
        fprintf(stderr, "%s:%d: Type %s does not exists!\n", "filename",
                parse_vars->token_buffer[1]->line, parse_vars->token_buffer[1]->text);
        parse_vars->has_error = true;
        return;
    }
    if (variable_array_add(parse_vars->variable_array, variable) == -1)
    {
        fprintf(stderr, "%s::%d: Name %s already exists!\n", "filename",
                parse_vars->token_buffer[2]->line, parse_vars->token_buffer[2]->text);
        parse_vars->has_error = true;
        return;
    }

    int16_t two_byte_type = (int16_t)variable.type;
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_CREATE_VAR,
              COMMAND_BYTES); // Command
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES],
              (char *)&(parse_vars->variable_array->head), ADDRESS_BYTES); // Var Address
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES],
              (char *)&two_byte_type, TYPE_BYTES); // Var type
    parse_vars->variable_array->head += get_size_of_type(variable.type);
    next_word(parse_vars);
}

static inline void assign(struct parse_vars *parse_vars)
{
    if (parse_vars->token_buffer[0]->type != NAME)
    {
        fprintf(stderr, "%s:%d: Assign value is not a name!\n", "file",
                parse_vars->token_buffer[0]->line);
        parse_vars->has_error = true;
        return;
    }

    int64_t index =
        variable_array_find(parse_vars->variable_array, parse_vars->token_buffer[0]->text);
    if (index < 0)
    {
        fprintf(stderr, "%s:%d: Variable %s does not exist to assign!\n", "filename",
                parse_vars->token_buffer[0]->line, parse_vars->token_buffer[0]->text);
        parse_vars->has_error = true;
        return;
    }

    if (parse_vars->token_buffer[2]->type == NUMBER)
    {
        enum variable_type type;
        if (strchr(parse_vars->token_buffer[2]->text, '.') != NULL)
        {
            type = F32;
        }
        else
        {
            type = I32;
        }
        if (parse_vars->variable_array->data[index].type != type)
        {
            fprintf(stderr, "%s:%d: Tried assigning %s to %s!\n", variable_type_to_string(type),
                    variable_type_to_string(parse_vars->variable_array->data[index].type));
            parse_vars->has_error = true;
            return;
        }

        int16_t two_byte_type = (int16_t)type;

        // Assign
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_ASSIGN, COMMAND_BYTES);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES],
                  (char *)&parse_vars->variable_array->data[index].position, ADDRESS_BYTES);
        set_bytes(
            &parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES],
            (char *)&two_byte_type, TYPE_BYTES);

        switch (type)
        {
        case BOOL:
            char value_bool;
            if (strcmp(parse_vars->token_buffer[2]->text, "true") == 0)
            {
                value_bool = 1;
            }
            else if (strcmp(parse_vars->token_buffer[2]->text, "false") == 0)
            {
                value_bool = 0;
            }
            else
            {
                return;
            }
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_bool, get_size_of_type(BOOL));
            break;
        case CHAR:
            char value_char = (char)atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_char, get_size_of_type(CHAR));
            break;
        case I8:
            int8_t value_int8 = (int8_t)atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_int8, get_size_of_type(I8));
            break;
        case I16:
            int16_t value_int16 = (int16_t)atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_int16, get_size_of_type(I16));
            break;
        case I32:
            int32_t value_int32 = atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_int32, get_size_of_type(I32));
            break;
        case I64:
            int64_t value_int64 = atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_int64, get_size_of_type(I64));
            break;
        case U8:
            uint8_t value_uint8 = (uint8_t)atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_uint8, get_size_of_type(U8));
            break;
        case U16:
            uint16_t value_uint16 = (uint16_t)atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_uint16, get_size_of_type(U16));
            break;
        case U32:
            uint32_t value_uint32 = atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_uint32, get_size_of_type(U32));
            break;
        case U64:
            uint64_t value_uint64 = atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_uint64, get_size_of_type(U64));
            break;
        case F32:
            float value_float = (float)atof(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_float, get_size_of_type(F32));
            break;
        case F64:
            double value_double = atof(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES +
                                          ADDRESS_BYTES + TYPE_BYTES],
                      (char *)&value_double, get_size_of_type(F64));
            break;
        case INVALID:
            break;
        default:
            break;
        }

        next_word(parse_vars);
    }
}

/// @brief
/// @param token_list
/// @param pre_init_variables Gets ownership, no need to free
/// @param pre_init_function_array Does not get ownership
/// @return
char *parse_tokens(const struct token_list *const token_list,
                   struct variable_array *pre_init_variables,
                   struct function_array *pre_init_function_array, bool is_strict)
{
    struct parse_vars parse_vars = {
        .function_array = function_array_init(),
        .cur_word = 0,
        .parsed = malloc(sizeof(char) * WORD_SIZE * BUFFER),
        .token_buffer = malloc(sizeof(struct token *) * 64),
        .token_buffer_count = 0,
        .has_error = false,
    };

    parse_vars.variable_array =
        pre_init_variables == NULL ? variable_array_init() : pre_init_variables;

    char inside_block = 0;

    for (size_t i = 0; i < token_list->count; i++)
    {
        if (token_list->tokens[i].type == BLOCK_START)
        {
            inside_block = 1;
        }
        if (token_list->tokens[i].type == BLOCK_END)
        {
            inside_block = 0;

            if (parse_vars.token_buffer[0]->type == FUNCTION)
            {
                if (parse_vars.token_buffer[1]->type != NAME)
                {
                    fprintf(stderr, "%s::%d: %s is not a valid function name!\n", __FILE__,
                            __LINE__, parse_vars.token_buffer[1]->text);
                    parse_vars.has_error = true;
                }
                if (parse_vars.token_buffer[2]->type != CALLER_START)
                {
                    fprintf(stderr, "%s::%d: No ( found!\n", __FILE__, __LINE__);
                    parse_vars.has_error = true;
                }

                struct function function = {
                    .name = parse_vars.token_buffer[1]->text,
                    .argument_count = 0,
                    .arguments = NULL,
                    .return_count = 0,
                    .return_type = NULL,
                };
                size_t function_block_start_index = 3;
                while (parse_vars.token_buffer[function_block_start_index]->type != CALLER_END)
                {
                    struct argument argument;
                    switch (function_block_start_index % 3)
                    {
                    case 0:
                        if (parse_vars.token_buffer[function_block_start_index]->type != NAME)
                        {
                            fprintf(stderr, "%s:%d: No argument type found!\n", __FILE__, __LINE__);
                        }
                        function.argument_count++;
                        argument.variable_type = variable_type_from_string(
                            parse_vars.token_buffer[function_block_start_index]->text);
                        break;
                    case 1:
                        if (parse_vars.token_buffer[function_block_start_index]->type != NAME)
                        {
                            fprintf(stderr, "%s:%d: No argument name found!\n", __FILE__, __LINE__);
                        }
                        argument.name = parse_vars.token_buffer[function_block_start_index]->text;
                        function.arguments = realloc(
                            function.arguments, sizeof(struct argument) * function.argument_count);
                        function.arguments[function.argument_count - 1] = argument;
                        break;
                    case 2:
                        if (parse_vars.token_buffer[function_block_start_index]->type != SEPARATOR)
                        {
                            fprintf(stderr, "%s:%d: No , found separating arguments!\n", __FILE__,
                                    __LINE__);
                        }
                        break;
                    }

                    function_block_start_index++;
                    if (function_block_start_index >= parse_vars.token_buffer_count)
                    {
                        fprintf(stderr, "%s:%d: No ) found!\n", __FILE__, __LINE__);
                    }
                }
                if (function_block_start_index % 3 != 2 && function_block_start_index != 3)
                {
                    fprintf(stderr, "%s:%d: Invalid function signature!\n", __FILE__, __LINE__);
                }

                if (parse_vars.token_buffer[function_block_start_index + 1]->type != BLOCK_START)
                {
                    fprintf(stderr, "%s:%d: No { found!\n", __FILE__, __LINE__);
                }

                struct token_list func_token_list;
                func_token_list.count =
                    parse_vars.token_buffer_count - (function_block_start_index + 2);
                func_token_list.tokens = parse_vars.token_buffer[function_block_start_index + 2];

                struct variable_array *func_variables = variable_array_init();
                func_variables->head +=
                    get_size_of_type(I32); // This value is reserved for the jump back
                for (size_t j = 0; j < function.argument_count; j++)
                {
                    struct variable argument = {
                        .name = function.arguments[j].name,
                        .type = function.arguments[j].variable_type,
                        .position = func_variables->head,
                    };
                    variable_array_add(func_variables, argument);
                    func_variables->head += get_size_of_type(argument.type);
                }

                char *func_contents = parse_tokens(&func_token_list, func_variables, NULL, false);

                // Jumps to after function definition
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_JUMP,
                          COMMAND_BYTES);
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES], "\0",
                          1); // Sets jump point as value
                // Jump location is set below
                size_t jump_command_position = parse_vars.cur_word;
                next_word(&parse_vars);

                function.call_position = parse_vars.cur_word;

                for (size_t j = 0; compare_bytes((char *)&func_contents[j * WORD_SIZE], C_EOP,
                                                 COMMAND_BYTES) == 0;
                     j++)
                {
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE],
                              func_contents + (j * WORD_SIZE), WORD_SIZE);
                    next_word(&parse_vars);
                }

                free(func_contents);

                // Jumps back to before function
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_JUMP,
                          COMMAND_BYTES);
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES], "\1",
                          1); // Sets jump point as reference
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES + 1],
                          "\x00\x00\x00\x00", ADDRESS_BYTES);
                next_word(&parse_vars);

                // Setting to where to jump to during definition
                int jump_location = parse_vars.cur_word;
                set_bytes(&parse_vars.parsed[jump_command_position * WORD_SIZE + COMMAND_BYTES + 1],
                          (char *)&jump_location, ADDRESS_BYTES);

                function_array_add(parse_vars.function_array, function);
            }
            else if (parse_vars.token_buffer[0]->type == BLOCK_START)
            {
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_BACKBLOCK,
                          COMMAND_BYTES);
                next_word(&parse_vars);
            }
            else
            {
                fprintf(stderr, "%s:%d: Block end found but where's the block start?\n", "file",
                        token_list->tokens[i].line);
                return NULL;
            }

            parse_vars.token_buffer_count = 0;
        }
        else if (token_list->tokens[i].type == ENDLINE && !inside_block)
        {
            if (parse_vars.token_buffer[0]->type == CREATE_VAR)
            {
                create_var(&parse_vars);
            }
            else if (parse_vars.token_buffer[0]->type == PRINT)
            {
                print(&parse_vars);
            }
            else if (parse_vars.token_buffer[0]->type == NAME)
            {
                if (parse_vars.token_buffer[1]->type == ASSIGN)
                {
                    assign(&parse_vars);
                }
                else if (parse_vars.token_buffer[1]->type == CALLER_START)
                {
                    int64_t function_index_return = function_array_find(
                        parse_vars.function_array, parse_vars.token_buffer[0]->text);

                    uint32_t function_index = (uint32_t)function_index_return;

                    uint32_t new_head = parse_vars.variable_array->head;

                    // Enters new stack
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_SETBLOCK,
                              COMMAND_BYTES);
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES],
                              (char *)&new_head, ADDRESS_BYTES);
                    next_word(&parse_vars);

                    int16_t two_byte_type = (int16_t)I32;
                    int32_t jump_position =
                        parse_vars.cur_word + 2 +
                        parse_vars.function_array->data[function_index].argument_count;

                    // Sets jump back
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_ASSIGN,
                              COMMAND_BYTES);
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES],
                              "\x00\x00\x00\x00", ADDRESS_BYTES); // Command
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES +
                                                 ADDRESS_BYTES],
                              (char *)&two_byte_type, TYPE_BYTES); // Var type
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES +
                                                 ADDRESS_BYTES + TYPE_BYTES],
                              (char *)&jump_position, ADDRESS_BYTES); // Jumps to after jump
                    next_word(&parse_vars);

                    uint32_t func_stack_head = get_size_of_type(I32);

                    for (size_t j = 0;
                         j < parse_vars.function_array->data[function_index].argument_count; j++)
                    {
                        if (j > 0 && parse_vars.token_buffer[1 + 2 * j]->type != SEPARATOR)
                        {
                            fprintf(stderr, "No separator found!");
                            parse_vars.has_error = true;
                        }

                        command_assign(&parse_vars,
                                       parse_vars.function_array->data[function_index]
                                           .arguments[j]
                                           .variable_type,
                                       func_stack_head, parse_vars.token_buffer[2 + 2 * j]->text);
                        func_stack_head += get_size_of_type(two_byte_type);
                    }

                    // Jumps to function
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_JUMP,
                              COMMAND_BYTES);
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES],
                              "\0", 1);
                    set_bytes(
                        &parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES + 1],
                        (char *)&parse_vars.function_array->data[function_index].call_position,
                        ADDRESS_BYTES);
                    next_word(&parse_vars);
                }
                else if (parse_vars.token_buffer[1]->type == ADD)
                {
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_OPERATION,
                              COMMAND_BYTES);
                    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES],
                              O_ADD, OPERATION_BYTES);
                    next_word(&parse_vars);
                }
            }
            else
            {
                fprintf(stderr, "%s:%d: Command does not exist!\n", "file",
                        parse_vars.token_buffer[0]->line);
                parse_vars.has_error = true;
            }

            parse_vars.token_buffer_count = 0;
        }
        else
        {
            parse_vars.token_buffer[parse_vars.token_buffer_count] = &token_list->tokens[i];
            parse_vars.token_buffer_count++;
        }
    }

    free(parse_vars.token_buffer);
    variable_array_free(parse_vars.variable_array);
    function_array_free(parse_vars.function_array);

    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_EOP, COMMAND_BYTES);
    if (parse_vars.has_error)
    {
        free(parse_vars.parsed);
        return NULL;
    }
    return parse_vars.parsed;
}
#undef BUFFER_SIZE