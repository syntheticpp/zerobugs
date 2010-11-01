#ifndef DEMANGLE_D_PARSER_H
#define DEMANGLE_D_PARSER_H 1

#include "config.h"
#include "string.h"

#define next_type		DD_(next_type)
#define interprete_template	DD_(interprete_template)
#define parse_real		DD_(parse_real)
#define parse_function		DD_(parse_function)
#define demangle_d		DD_(demangle_d)

char* next_type(string_t dest, char* raw, int is_nested);

void interprete_template(string_t dest, char* raw);

char* parse_real(string_t dest, char* raw);

char* parse_function(string_t dest, char* raw, char* name, int is_nested, int has_this_pointer);

#endif /* DEMANGLE_D_PARSER_H */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
