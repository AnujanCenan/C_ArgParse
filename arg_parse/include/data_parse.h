#ifndef DATA_PARSE_H
#define DATA_PARSE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_type.h"

int parse_int(String s);
bool parse_bool(String s);
double parse_double(String s);
float parse_float(String s);
String parse_string(String s);

#endif