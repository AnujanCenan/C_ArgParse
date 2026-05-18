#include "arg_parse.h"

// Base compilation command
// (assuming cwd == C_ArgParse/arg_parse)
// clang -g  -Iinclude/ -I../string_hashing/include ../string_hashing/src/*.c ./src/*.c  main.c



int main(int argc, String argv[])
{
    Arg_Parser* parser = arg_parser_init();
    arg_parser_add_pos_arg(parser, "filename", STRING);
    arg_parser_add_flag(parser, "-a", "--address", NULL, BOOL, true);
    arg_parser_add_flag(parser, "-b", NULL, "13", INT, true);
    arg_parser_add_flag(parser, "-c", NULL, NULL, BOOL, true);

    arg_parser_parse(parser, argc, argv);

    print_arg_value(parser, "filename");
    print_arg_value(parser, "-a");
    print_arg_value(parser, "-b");
    print_arg_value(parser, "-c");

    arg_parser_free(parser);


    return 0;
}