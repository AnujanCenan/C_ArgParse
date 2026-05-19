#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_type.h"


int get_digit(char c)
{
    switch (c)
    {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        default:
            fprintf(stderr, "data_parsing::get_digit: Given character %d is not a digit\n", c);
            exit(1);

    }
}

int parse_int(String s)
{
    int res = 0;
    for (char* c = s; *c != '\0'; ++c)
    {
        int dig = get_digit(*c);
        res = res * 10 + dig;
    }

    return res;
}

bool parse_bool(String s)
{
    if (strcmp(s, "true") == 0) return true;
    if (strcmp(s, "false") == 0) return false;

    fprintf(stderr, "data_parsing::parse_bool: given string %s is not true or false\n", s);
    exit(1);
}



double parse_double(String s)
{
    int dot_pos = -1;
    double integral = 0;
    int last_idx = 0;
    for (int i = 0; s[i] != '\0'; ++i)
    {
        if (s[i] == '.' && dot_pos == -1)
        {
            dot_pos = i;
        } else if (s[i] == '.' && dot_pos != -1)
        {
            fprintf(stderr, "data_parsing::parse_double: given string %s has more than one . in it\n", s);
            exit(1);
        } else if (dot_pos == -1)
        {
            int dig = get_digit(s[i]);
            integral = integral * 10 + dig;
        }
        last_idx = i;
    }

    if (dot_pos == -1) return integral;

    printf("integral value = %f; last_idx = %d\n", integral, last_idx);
    
    double frac = 0;
    for (int i = last_idx; s[i] != '.'; --i)
    {
        double dig = (double) get_digit(s[i]) / 10;
        frac = (frac / 10) + dig;
    }

    return integral + frac;
}

float parse_float(String s)
{
    return (float) parse_double(s);
}


String parse_string(String s)
{
    return s;
}


int main()
{
    int x = parse_int("123");
    printf("int = %d\n", 123);

    bool b = parse_bool("true");
    printf("bool = %d\n", b);

    float f = parse_float("841.3904");
    printf("float = %f\n", f);

    double d = parse_double("1432.2131243");
    printf("double = %lf\n", d);

    String s = parse_string("sdalfjasd;fjd;adfsjjfio;xajsdfiocewfunopv");
    printf("string = %s\n", s);
    return 0;
}