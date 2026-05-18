#include "string_hashing.h"

typedef enum Arg_Type Arg_Type;
typedef char* String;
typedef struct Argument Argument;
typedef struct Arg_Parser Arg_Parser;


enum Arg_Type
{
    BOOL = 'b',
    STRING = 's',
    INT = 'i',
    FLOAT = 'f',
    DOUBLE = 'd'
};

struct Argument
{
    String value;           // Assumes on the stack
    String default_value;   // Assumes on the stack
    Arg_Type type;

    int ref_count;
    bool optional;
};

struct Arg_Parser
{
    String_Hash_Table* flag_values;     // maps Strings to Arguments

    String_Hash_Table* map_arg_pos;     // maps Strings to ints
    Argument** positional_arguments;

    int num_pos_args;
    int curr_pos_arg;
};


/**
 * Initialises and returns (the address of) an Arg_Parse object to use for parsing
 * command line arguments.
 */
Arg_Parser* arg_parser_init();

/**
 * Adds a positional argument to the parser. Note that positional arguments should
 * not have a - as the first character.
 * - parser: the Arg_Parser object being used 
 * - arg_name: the name of the positional argument (e.g. num_lines, filename)
 * - - type: the argument's type (see Arg_Type enum) for options
 */
void arg_parser_add_pos_arg(Arg_Parser* parser, String arg_name, Arg_Type type);

/**
 * Adds a flag (e.g. -x or --filename) to the parser
 * - short_option: the short string option for the flag; should have a single '-'
 *      as a prefix
 * - long_option: the long string option for hte flag; should have a double '--'
 *      as a prefix
 * - default_value: the string version of the default value for this flag;
 *      e.g. if the flag is for an integer and the default value is 3, then pass in "3"
 * - type: the argument's type (see Arg_Type enum) for options
 * - optional: whether the flag is optional or not; note that when type == BOOL,
 *      optional should be set to true
 */
void arg_parser_add_flag(
    Arg_Parser* parser, 
    String short_option, String long_option, 
    String default_value,
    Arg_Type type, 
    bool optional
);

void arg_parser_parse(Arg_Parser* parser, int argc, String argv[]);

/**
 * Given a particular arg_name (e.g. "-f", "--xtreme", "num_lines"), returns the 
 * Argument object that corresponds to that argument. Note that from this argument
 * object, you can see the String version of the value of this argument and the 
 * type that this argument has.
 */
Argument* arg_parser_get(Arg_Parser* parser, String arg_name);

/**
 * Frees all memory associated with a particular Arg_Parser object
 */
void arg_parser_free(Arg_Parser* parser);


/**
 * Given a particular arg_name (e.g. "-f", "--xtreme", "num_lines"), prints the
 * value that the user gave to correspond to that argument.
 */
void print_arg_value(Arg_Parser* parser, String arg_name);