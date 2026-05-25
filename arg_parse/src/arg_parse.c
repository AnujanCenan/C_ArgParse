#include "arg_parse.h"

#define MAX_POS_ARGS 10

#define TRUE_LIT "1"
#define FALSE_LIT "0"

/// Function declarations

void free_argument_item(String_HT_Item* item);
void free_int_item(String_HT_Item* item);

/**
 * Helper function for the parsing process
 * - parser = the parser object used for the parsing process
 * - flag = the actual flag you are up to (e.g. "-a", "--xtreme")
 * - argc, argv = the same argc and argv obtained from the main function
 * - index: the index that points to the current flag in argv
 *          i.e. argv[index] == flag
 */
Argument* add_flag_value(
    Arg_Parser* parser, 
    String flag, 
    int argc, String argv[], 
    int index
);


/// Function Definitions

void free_argument_item(String_HT_Item* item)
{
    Argument* arg = (Argument*) item->value;
    arg->ref_count--;
    if (arg->ref_count == 0)
    {
        free(arg);
    }
    free(item);
}

void free_int_item(String_HT_Item* item)
{
    free(item->value);
    free(item);
}


Arg_Parser* arg_parser_init()
{
    Arg_Parser* parser = malloc(sizeof(Arg_Parser));
    parser->flag_values = string_hashtable_init(free_argument_item);

    parser->map_arg_pos = string_hashtable_init(free_int_item);
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
    arg->default_value = NULL;
    arg->ref_count = 1;

    int* pos = malloc(sizeof(int));
    *pos = parser->num_pos_args;

    if (string_hashtable_get(parser->map_arg_pos, arg_name))
    {
        fprintf(stderr, "Arg_Parser::arg_parser_add_pos_arg: Duplicate argument detected\n");
        exit(1);
    }

    string_hashtable_add(&(parser->map_arg_pos), arg_name, pos);
    parser->positional_arguments[parser->num_pos_args++] = arg;
}

// Note that default_value is allowed to be NULL
void arg_parser_add_flag(
    Arg_Parser* parser, 
    String short_option, String long_option, 
    String default_value,
    Arg_Type type, 
    bool optional
)
{
    if (!short_option && !long_option)
    {
        fprintf(stderr, "Arg_Parser::arg_parser_add_flag: When using arg_parser_add_flag, at least one of short_option and long_option should be non-NULL\n");
        exit(1);
    }

    if (type == BOOL && !optional)
    {
        fprintf(stderr, "Arg_Parser::arg_parser_add_flag: WARNING -- setting a boolean flag as NOT optional renders the flag redundant\n");
    }

    Argument* arg = malloc(sizeof(Argument));
    arg->value = NULL;
    arg->optional = optional;
    arg->type = type;
    arg->default_value = default_value;
    arg->ref_count = 0;

    if (short_option)
    {
        if (string_hashtable_get(parser->flag_values, short_option))
        {
            fprintf(stderr, "ArgParse::arg_parser_add_flag: Duplicate argument detected\n");
            exit(1);
        }
        arg->ref_count++;
        string_hashtable_add(&(parser->flag_values), short_option, arg);
    }

    if (long_option)
    {
        if (string_hashtable_get(parser->flag_values, long_option))
        {
            fprintf(stderr, "ArgParse::arg_parser_add_flag: Duplicate argument detected\n");
            exit(1);
        }
        arg->ref_count++;
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
        arg->value = TRUE_LIT; 
    } else if (index + 1 >= argc)
    {
        fprintf(stderr, "Arg_Parser::add_flag_value: Argument index=%d requires a value\n", index);
        exit(1);
    } else
    {
        arg->value = argv[index + 1];
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


    for (int i = 0; i < parser->flag_values->capacity; ++i)
    {
        if (!parser->flag_values->ht[i] || parser->flag_values->ht[i]->tombstone) continue;
        Argument* arg =  (Argument*) parser->flag_values->ht[i]->value;
        if (!arg->value && arg->type == BOOL)
        {
            arg->value = FALSE_LIT;
        } else if (!arg->value && arg->default_value)
        {
            arg->value = arg->default_value;
        } else if (!arg->value && !arg->optional)
        {
            fputs("Argument given is not optional\n\t", stderr);
            fputs(parser->flag_values->ht[i]->key, stderr);
            fputs("\n", stderr);
            exit(1);
        }
    }


}

Argument* arg_parser_get(Arg_Parser* parser, String arg_name)
{
    Argument* arg = NULL;
    if (arg_name[0] == '-')
    {
        arg = (Argument*) string_hashtable_get(parser->flag_values, arg_name);
        if (!arg)
        {
            fprintf(stderr, "ArgParser::arg_parser_get: it appears argument %s does not exist\n", arg_name);
            exit(1);
        }
    } else
    {
        int* pos = (int*) string_hashtable_get(parser->map_arg_pos, arg_name);
        if (!pos)
        {
            fprintf(stderr, "ArgParser::arg_parser_get: it appears argument %s does not exist\n", arg_name);
            exit(1);
        }
        arg = parser->positional_arguments[*pos];
    }
    
    if (arg->value == NULL) 
    {
        printf("Arg_Parse::arg_parser_get: WARNING: argument %s has a NULL value \
            did you call arg_parser_parse?\n", arg_name);
    }

    return arg;
}

// Debugging only
void print_arg_value(Arg_Parser* parser, String arg_name)
{
    Argument* arg = arg_parser_get(parser, arg_name);
    printf("%s == %s\n", arg_name, arg->value);
}

void arg_parser_free(Arg_Parser* parser)
{

    string_hashtable_free(parser->flag_values);
    string_hashtable_free(parser->map_arg_pos);

    for (int i = 0; i < parser->num_pos_args; ++i)
    {
        free(parser->positional_arguments[i]);
    }

    free(parser->positional_arguments);
    free(parser);
}