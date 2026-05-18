#include "string_hashing.h"

#define MAX_POS_ARGS 10

enum Arg_Type
{
    BOOL = 'b',
    STRING = 's',
    INT = 'i',
    FLOAT = 'f',
    DOUBLE = 'd'
};


typedef enum Arg_Type Arg_Type;
typedef char* String;
typedef struct Argument Argument;
typedef struct Arg_Parser Arg_Parser;

static String true_lit = "true";


struct Argument
{
    String value;
    Arg_Type type;
    bool optional;
};

// struct Flag_Parser

// struct Positional_Arg_Parser

struct Arg_Parser
{
    String_Hash_Table* flag_values;
    
    String_Hash_Table* long_to_short;       // translate long flag to short flag
    String_Hash_Table* short_to_long;       // translate short flag to long flag


    String_Hash_Table* map_arg_pos;
    Argument** positional_arguments;
    int num_pos_args;
    int curr_pos_arg;
};



Arg_Parser* arg_parser_init()
{
    Arg_Parser* parser = malloc(sizeof(Arg_Parser));
    parser->flag_values = string_hashtable_init();
    parser->short_to_long = string_hashtable_init();
    parser->long_to_short = string_hashtable_init();

    parser->map_arg_pos = string_hashtable_init();
    parser->positional_arguments = malloc(MAX_POS_ARGS * sizeof(Argument*));
    
    parser->num_pos_args = 0;
    parser->curr_pos_arg = 0;

    return parser;
}

void arg_parser_add_pos_arg(Arg_Parser* parser, String arg_name, Arg_Type type)
{
    if (!arg_name)
    {
        fprintf(stderr, "Arg_Parser::arg_parser_add_pos_arg: arg_name cannot be null\n");
        exit(1);
    }

    if (arg_name[0] == '-')
    {
        fprintf(stderr, "Arg_Parser::arg_parser_add_pos_arg: positional argument cannot start with a -\n");
        exit(1); 
    }
    
    Argument* arg = malloc(sizeof(Argument));
    arg->value = NULL;
    arg->optional = false;
    arg->type = type;

    int* pos = malloc(sizeof(int));
    *pos = parser->num_pos_args;

    string_hashtable_add(&(parser->map_arg_pos), arg_name, pos);
    parser->positional_arguments[parser->num_pos_args++] = arg;
}

void arg_parser_add_flag(
    Arg_Parser* parser, 
    String short_option, String long_option, 
    Arg_Type type, 
    bool optional
)
{
    if (!short_option && !long_option)
    {
        fprintf(stderr, "Arg_Parser::arg_parser_add_flag: When using arg_parser_add_flag, at least one of short_option and long_option should be non-NULL\n");
        exit(1);
    }

    Argument* arg = malloc(sizeof(Argument));
    arg->value = NULL;
    arg->optional = optional;
    arg->type = type;

    if (short_option && long_option)
    {
        string_hashtable_add(&(parser->long_to_short), short_option, strdup(long_option));
        string_hashtable_add(&(parser->long_to_short), long_option, strdup(short_option));
    }

    if (short_option)
    {
        string_hashtable_add(&(parser->flag_values), short_option, arg);
    }

    if (long_option)
    {
        string_hashtable_add(&(parser->flag_values), long_option, arg);
    }
}

// index points to the particular index of argv you are up to
Argument* add_flag_value(Arg_Parser* parser, String flag, int argc, String argv[], int index)
{
    if (!flag) return NULL;

    Argument* arg = (Argument*) string_hashtable_get(parser->flag_values, flag);
    if (!arg) return NULL;

    if (arg->type == BOOL)
    {
        arg->value = strdup("true");
    } else if (index + 1 >= argc)
    {
        fprintf(stderr, "Argument %d requires a value\n", index);
        exit(1);
    } else
    {
        arg->value = strdup(argv[index + 1]);
    }

    return arg;
}

void arg_parser_parse(Arg_Parser* parser, int argc, String argv[])
{
    for (int i = 1; i < argc; ++i)      // skipping i = 0 as this refers to the program name
    {
        if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            if (argv[i][2] == '\0')
            {
                fprintf(stderr, "ArgParse does not support argument --\n");
                exit(1);
            }

            Argument* arg = add_flag_value(parser, argv[i], argc, argv, i);
            if (!arg)
            {
                fprintf(stderr, "Argument %s not recognised\n", argv[i]);
                exit(1);
            }

            // String short_version = (String) string_hashtable_get(parser->long_to_short, argv[i]);
            // add_flag_value(parser, short_version, argc, argv, i);

            if (arg->type != BOOL) ++i;
        } else if (argv[i][0] == '-')
        {
            if (argv[i][1] == '\0')
            {
                fprintf(stderr, "ArgParse does not support argument -\n");
                exit(1);
            }

            if (argv[i][2] == '\0')
            {
                Argument* arg = add_flag_value(parser, argv[i], argc, argv, i);
                if (!arg)
                {
                    fprintf(stderr, "Argument %s not recognised\n", argv[i]);
                    exit(1);
                }

                // String long_version = (String) string_hashtable_get(parser->short_to_long, argv[i]);
                // add_flag_value(parser, long_version, argc, argv, i);
                if (arg->type != BOOL) ++i;

            } else
            {
                for (char* c = &(argv[i][1]); *c != '\0'; ++c)
                {
                    char flag[] = {'-', *c, '\0'};
                    String heap_flag = strdup(flag);

                    Argument* arg = add_flag_value(parser, heap_flag, argc, argv, i);
                    free(heap_flag);
                    if (!arg)
                    {
                        fprintf(stderr, "Argument %s not recognised\n", argv[i]);
                        exit(1);
                    }

                    if (arg->type != BOOL)
                    {
                        fprintf(stderr, "ArgParse assumes arguments of the form -abc conists of only boolean flags\n");
                        exit(1);
                    }

                    // String long_version = (String) string_hashtable_get(parser->short_to_long, heap_flag);
                    // add_flag_value(parser, long_version, argc, argv, i);
                }
            }
        } else
        {
            String pos_arg = argv[i];
            if (parser->curr_pos_arg >= parser->num_pos_args)
            {
                fprintf(stderr, "It seems we have provided too many positional arguments\n");
                exit(1);
            }
            parser->positional_arguments[parser->curr_pos_arg++]->value = argv[i];
        }
    }

    if (parser->curr_pos_arg < parser->num_pos_args)
    {
        fprintf(stderr, "It seems we have not provided enough positional arguments\n");
        exit(1);
    }
}

Argument* arg_parser_get(Arg_Parser* parser, String arg_name)
{
    if (arg_name[0] == '-')
    {
        Argument* arg = (Argument*) string_hashtable_get(parser->flag_values, arg_name);
        if (!arg)
        {
            fprintf(stderr, "ArgParser::arg_parser_get: it appears argument %s does not exist\n", arg_name);
            exit(1);
        }

        return arg;
    } else
    {
        int* pos = (int*) string_hashtable_get(parser->map_arg_pos, arg_name);
        if (!pos)
        {
            fprintf(stderr, "ArgParser::arg_parser_get: it appears argument %s does not exist\n", arg_name);
            exit(1);
        }
        Argument* arg = parser->positional_arguments[*pos];
        return arg;
    }
}


int main(int argc, String argv[])
{
    Arg_Parser* parser = arg_parser_init();
    return 0;
}