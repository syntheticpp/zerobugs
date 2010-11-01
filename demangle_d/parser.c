/*
 * demangle_d - pluggable D de-mangler
 * Copyright (C) 2006-2007 Thomas Kuehne <thomas@kuehne.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent modules,
 * and to copy and distribute the resulting executable under terms of your
 * choice, provided that you also meet, for each linked independent module,
 * the terms and conditions of the license of that module. An independent
 * module is a module which is not derived from or based on this library. If
 * you modify this library, you may extend this exception to your version of
 * the library, but you are not obligated to do so. If you do not wish to do
 * so, delete this exception statement from your version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "parser.h"
#include "util.h"

/* Parse until the end of the next type and return the pointer to the first
   uninterpreted charachter or NULL.  Set is_nested if a nested type
   (e.g. index of AA or template/function parameter) is parsed.  */
char*
next_type(dest, source, is_nested)
	string_t dest; char* source; int is_nested;
{
    int has_this_pointer = 0;

    if (!source || !source[0])
	return NULL;

    if ((source[0] == '_') && (source[1] == 'D') && xisdigit(source[2])
	    && (source[2] != '0'))
      {
	string_t tmp;
	tmp = new_string();
	source = next_type(tmp, source + 2, 0);
	if (dest->used && (dest->str[dest->used] != '.'))
	    append_c(dest, '.');
	append_n(dest, tmp->str, tmp->used);
	xfree(tmp->str);
	xfree(tmp);
	return source;
      }

Lreevaluate:
    switch(source[0])
      {
	case 'M': /* this pointer since DMD-0.177 */
	    has_this_pointer = 1;
	    source++;
	    goto Lreevaluate;
	case 'v':
	    nestpend(dest, "void", is_nested);
	    source += 1;
	    break;
	case 'Z': /* runtime internals since DMD-0.176 */
	    nestpend(dest, "const void", is_nested);
	    source += 1;
	    break;
	case 'b':
	    nestpend(dest, "bool", is_nested);
	    source += 1;
	    break;
	case 'g':
	    nestpend(dest, "byte", is_nested);
	    source += 1;
	    break;
	case 'h':
	    nestpend(dest, "ubyte", is_nested);
	    source += 1;
	    break;
	case 's':
	    nestpend(dest, "short", is_nested);
	    source += 1;
	    break;
	case 't':
	    nestpend(dest, "ushort", is_nested);
	    source += 1;
	    break;
	case 'i':
	    nestpend(dest, "int", is_nested);
	    source += 1;
	    break;
	case 'k':
	    nestpend(dest, "uint", is_nested);
	    source += 1;
	    break;
	case 'l':
	    nestpend(dest, "long", is_nested);
	    source += 1;
	    break;
	case 'm':
	    nestpend(dest, "ulong", is_nested);
	    source += 1;
	    break;
	case 'f':
	    nestpend(dest, "float", is_nested);
	    source += 1;
	    break;
	case 'd':
	    nestpend(dest, "double", is_nested);
	    source += 1;
	    break;
	case 'e':
	    nestpend(dest, "real", is_nested);
	    source += 1;
	    break;
	case 'o':
	    nestpend(dest, "ifloat", is_nested);
	    source += 1;
	    break;
	case 'p':
	    nestpend(dest, "idouble", is_nested);
	    source += 1;
	    break;
	case 'j':
	    nestpend(dest, "ireal", is_nested);
	    source += 1;
	    break;
	case 'q':
	    nestpend(dest, "cfloat", is_nested);
	    source += 1;
	    break;
	case 'r':
	    nestpend(dest, "cdouble", is_nested);
	    source += 1;
	    break;
	case 'c':
	    nestpend(dest, "creal", is_nested);
	    source += 1;
	    break;
	case 'a':
	    nestpend(dest, "char", is_nested);
	    source += 1;
	    break;
	case 'u':
	    nestpend(dest, "wchar", is_nested);
	    source += 1;
	    break;
	case 'w':
	    nestpend(dest, "dchar", is_nested);
	    source += 1;
	    break;
	case 'x':
	    prepend(dest, "const ");
	    source = next_type(dest, source+1, is_nested);
	    break;
	case 'y':
	    prepend(dest, "invariant ");
	    source = next_type(dest, source+1, is_nested);
	    break;

	case 'A': /* dynamic array */
	    if (!is_nested)
		prepend(dest, "[] ");
	    source = next_type(dest, source+1, is_nested);
	    if (is_nested)
		append(dest, "[]");
	    break;

	case 'G':  /* static array */
	  {
	    char* start;
	    char* end;
	    start = ++source;
	    end = start;

	    while (xisdigit(*end))
		end++;
	
	    if (!is_nested)
	      {
		prepend(dest, "] ");
		prepend_n(dest, start, end-start);
		prepend(dest, "[");
	      }
	    source = next_type(dest, end, is_nested);
	    if (is_nested)
	      {
		append_c(dest, '[');
		append_n(dest, start, end-start);
		append_c(dest, ']');
	      }
	    break;
	  }

	case 'H': /* associative array */
	  {
	    string_t aa;
	    aa = new_string();
	    source = next_type(aa, source+1, 1);
	    prepend(aa, "[");
	    append_c(aa, ']');
	    source = next_type(aa, source, 0);
	    nestpend(dest, aa->str, is_nested);
	    xfree(aa->str);
	    xfree(aa);
	    break;
	  }

	case 'D': /* delegate */
	  {
	    string_t sig;
	    sig = new_string();
	    source = parse_function(sig, source+1, NULL, 0, has_this_pointer);
	    nestpend_n(dest, sig->str, sig->used, is_nested);
	    xfree(sig->str);
	    xfree(sig);
	    break;
	  }

	case 'P': /* pointer */
	    if ((source[1] == 'F') || (source[1]=='U') || (source[1]=='W')
		    || (source[1]=='V') || (source[1]=='R'))
	      {
		/* function */
		string_t sig;
		sig = new_string();
		source = parse_function(sig, source+1, "", 0, has_this_pointer);
		nestpend_n(dest, sig->str, sig->used, is_nested);
		xfree(sig->str);
		xfree(sig);
	      }
	    else
	      {
		/* 'normal' type */
		if (!is_nested)
		    prepend(dest, "* ");
		source = next_type(dest, source+1, is_nested);
		if (is_nested)
		    append(dest, " *");
	      }
	    break;

	case 'J': /* out */
	    append(dest, "out ");
	    source = next_type(dest, source+1, 1);
	    break;

	case 'K': /* inout */
	    append(dest, "inout ");
	    source = next_type(dest, source+1, 1);
	    break;
	
	case 'L': /* lazy */
	    append(dest, "lazy ");
	    source = next_type(dest, source+1, 1);
	    break;

	case 'C': /* class */
	case 'S': /* struct */
	case 'E': /* enum */
	case 'T': /* typedef */
	  {
#ifdef DEMANGLE_D_VERBOSE
	    char tmp;
	    tmp = source[0];
#endif /* DEMANGLE_D_VERBOSE */
	    if (!is_nested)
	      {
		string_t sig;
		sig = new_string();
		source = next_type(sig, source+1, 0);
		append_c(sig, ' ');
#ifdef DEMANGLE_D_VERBOSE
		switch (tmp)
		  {
		    case 'C':
			prepend(sig, "class ");
			break;
		    case 'S':
			prepend(sig, "struct ");
			break;
		    case 'E':
			prepend(sig, "enum ");
			break;
		    case 'T':
			prepend(sig, "typedef ");
			break;
		  }
#endif /* DEMANGLE_D_VERBOSE */
		prepend_n(dest, sig->str, sig->used);
		xfree(sig->str);
		xfree(sig);
	      }
	    else
	      {
#ifdef DEMANGLE_D_VERBOSE
		switch (tmp)
		  {
		    case 'C':
			append(dest, "class ");
			break;
		    case 'S':
			append(dest, "struct ");
			break;
		    case 'E':
			append(dest, "enum ");
			break;
		    case 'T':
			append(dest, "typedef ");
			break;
		  }
#endif /* DEMANGLE_D_VERBOSE */
		source = next_type(dest, source+1, 1);
	      }
	    break;
	  }

	case '1': /* qualified name */
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  {
	    int first_round;
	    first_round = 1;

	    while (source && xisdigit(source[0]) && (source[0] != '0'))
	      {
		long int len;
		len = xstrtol_10(source, &source);

		if (!first_round)
		    append_c(dest, '.');
		else
		    first_round = 0;
		
		if (len >= 5 && (source[0] == '_') && (source[1] == '_')
			&& (source[2] == 'T') && xisdigit(source[3])
			&& (source[3] != '0'))
		  {
		    /* template */
		    char* template;
		    template = xstrndup(source + 3, len-3);
		    interprete_template(dest, template);
		    xfree(template);
		  }
		else if((len == 5) && (xstrncmp(source, "_ctor", len) == 0))
		    append(dest, "this");
		else if((len == 5) && (xstrncmp(source, "_dtor", len) == 0))
		    append(dest, "~this");
		else if((len == 11)
			&& (xstrncmp(source, "_staticCtorFZv", len + 3) == 0))
		  {
		    prepend(dest, "static void ");
		    append(dest, "this()");
		    source = source + 11 + 3;
		    break;
		  }
		else if((len == 11)
			&& (xstrncmp(source, "_staticDtorFZv", len + 3) == 0))
		  {
		    prepend(dest, "static void ");
		    append(dest, "~this()");
		    source = source + 11 + 3;
		    break;
		  }
		else
		    /* plain identifier part */
		    append_n(dest, source, len);

		source += len;
	      }
	    if (!is_nested)
		source = next_type(dest, source, 0);
	    break;
	  }

	case 'F': /* D function */
	case 'U': /* C function */
	case 'W': /* Windows function */
	case 'V': /* Pascal function */
	case 'R': /* C++ function */
	    if (!is_nested)
	      {
		string_t id;
		id = new_string();
		append_n(id, dest->str, dest->used);
		dest->used = 0;
		dest->str[0] = '\x00';
		source = parse_function(dest, source, id->str, 0, has_this_pointer);
		xfree(id->str);
		xfree(id);
	      }
	    else
		source = parse_function(dest, source, "", 1, has_this_pointer);
	    break;

	case 'B': /* Tuple */
	  {
	    append(dest, "Tuple[");
	    char* end;
	    long int elements = strtol(source + 1, &end, 10);
	    source = end;
	    string_t element;
	    element = new_string();
	    while (elements > 1) {
		elements--;    
		element->used = 0;
		element->str[0] = '\x00';
		source = next_type(element, source, 1);
		append(dest, element->str);
		append(dest, ", ");
	    }
	    if (elements == 1)
	      {
		element->used = 0;
		element->str[0] = '\x00';
		source = next_type(element, source, 1);
		append(dest, element->str);
	      }
	    append(dest, "]");
	    xfree(element->str);
	    xfree(element);
	    break;
	  }

	case '.':
	    if('1' <= source[1] && source[1] <= '9'){
		/* ignore trailing junk generated by GDC for nested functions
		 * http://d.puremagic.com/issues/show_bug.cgi?id=375
		 */
		source = NULL;
		break;
	    }

	    /* fall through */

	default:
	    append(dest, " @bug@[2]{");
	    append(dest, source);
	    append_c(dest, '}');
	    source = NULL;
	    break;
      }

    return source;
}

/* Parse a "real" template parameter and return a pointer to the first not
   interpreted character.  */
char*
parse_real(dest, source)
	string_t dest; char* source;
{
    char buffer[64];
    int index;
    int digit_count;
    long double result;
    char* end;

    if (strncmp(source, "NAN", 3) == 0)
      {
	append(dest, "NAN");
	return source + 3;
      }
    else if (strncmp(source, "INF", 3) == 0)
      {
	append(dest, "INF");
	return source + 3;
      }
    else if (strncmp(source, "NINF", 4) == 0)
      {
	append(dest, "-INF");
	return source + 4;
      }

    index = 0;
    digit_count = 0;

    if (*source == 'N')
      {
	buffer[index++] = '-';
	source++;
      }

    buffer[index++] = '0';
    buffer[index++] = 'x';
    
    while (xisxdigit(*source))
      {
	buffer[index++] = *source;
	if (1 == digit_count++)
	    buffer[index++] = '.';	 
	source++;
      }

    if (*source == 'P')
      {
	buffer[index++] = *source;
	source++;
      }
    else
      {
	append(dest, " @bug@[3]{");
	append(dest, source);
	append_c(dest, '}');
	return NULL;
      }
	
    if (*source == 'N')
      {
	buffer[index++] = '-';
	source++;
      }
    else
      {
	buffer[index++] = '+';      
      }
    
    while (xisxdigit(*source))
      {
	buffer[index++] = *source;
	source++;
      }

    buffer[index] = 0;

    result = 0;
    result = strtold(buffer, &end);
    if (end && end == buffer + index)
      {
        snprintf(buffer, sizeof(buffer), "%Lf", result);
	append(dest, buffer);
      }
    else
      {
	perror("strtold");      
	append(dest, buffer);
      }


    return source;
}

/* Parse a function - including arguments and return type - and
   return a pointer to the first not interpreted character.  */
char*
parse_function(dest, source, name, is_nested, has_this_pointer)
	string_t dest; char* source; char* name; int is_nested;
	int has_this_pointer;
{
    string_t fn_return;
    string_t fn_param;

    fn_return = new_string();
    fn_param = new_string();

    source++;

    /* params */
    if (source[0] != 'Z')
      {
	if (source[0] == 'Y')
	  {
	    append(fn_param, "...");
	    goto var_arg_param;
	  }
	source = next_type(fn_param, source, 1);
	while (source && source[0] && source[0]!='Z')
	  {
	    if (source[0] == 'Y')
	      {
		append(fn_param, ", ...");
		goto var_arg_param;
	      }
	    else if(source[0] == 'X')
	      {
		append(fn_param, " ...");
		goto var_arg_param;
	      }
	    append(fn_param, ", ");
	    source = next_type(fn_param, source, 1);
	  }
      }

    /* return type */
    if (source && source[0] == 'Z')
var_arg_param:
	source = next_type(fn_return, source + 1, 1);

    /* output */
    if (name && name[0])
	if (! is_nested)
	  {
	    prepend(dest, " ");
	    prepend_n(dest, fn_return->str, fn_return->used);
	    append(dest, name);
	  }
	else
	  {
	    append_n(dest, fn_return->str, fn_return->used);
	    append_c(dest, ' ');
	    append(dest, name);
	  }
    else if(name)
      {
	append_n(dest, fn_return->str, fn_return->used);
	append(dest, " function");
      }
    else
      {
	append_n(dest, fn_return->str, fn_return->used);
	append(dest, " delegate");
      }

    if (fn_param->used)
      {
	append_c(dest, '(');
	append_n(dest, fn_param->str, fn_param->used);
	if (has_this_pointer)
	  {
	    append(dest, ", this");
	  }
	append_c(dest, ')');
      }
    else if(has_this_pointer)
	append(dest, "(this)");
    else
	append(dest, "()");

    xfree(fn_return->str);
    xfree(fn_return);
    xfree(fn_param->str);
    xfree(fn_param);

    return source;
}

/* Interprete the NULL terminated template symbol.  */
void
interprete_template(dest, raw)
	string_t dest; char* raw;
{
    char* tmp;
    long int dataLen;
    int first_arg;

    first_arg = 1;

    /* id */
    while (xisdigit(raw[0]) && (raw[0] != '0'))
      {
	long int len;
	len = xstrtol_10(raw, &raw);
	append_n(dest, raw, len);
	raw += len;
      }
    append(dest, "!(");

    /* arguments */
    while (raw && raw[0])
      {
	if (raw[0] == 'T')
	  {
	    /* type parameter */
	    raw++;
	    if (!first_arg)
		append(dest, ", ");
	    else
		first_arg = 0;
	    raw = next_type(dest, raw, 1);
	  }
	else if(raw[0] == 'V')
	  {
	    /* value parameter */
	    if (!first_arg)
		append(dest, ", ");
	    else
		first_arg = 0;
	    raw = next_type(dest, raw + 1, 1);
            if (!raw)
                break;

	    append_c(dest, ' ');
            
	    if (xisdigit(raw[0]))
	      {
		/* positive integer */
integer_arg:
		tmp = raw;
		while (xisdigit(raw[0]))
		    raw++;
		append_n(dest, tmp, raw-tmp);
	      }
	    else if(raw[0] == 'N')
	      {
		/* negative integer */
		raw++;
		append_c(dest, '-');
		goto integer_arg;
	      }
	    else if(raw[0] == 'e')
		/* float */
		raw = parse_real(dest, raw+1);
	    else if(raw[0] == 'c')
	      {
		/* complex float */
		raw = parse_real(dest, raw+1);
		append(dest, " + ");
		raw = parse_real(dest, raw);
		append_c(dest, 'i');
	      }
	    else if(raw[0] == 'n')
	      {
		append(dest, "null");
		raw++;
	      }
	    else if((raw[0] == 'a') || (raw[0] == 'w') || (raw[0] == 'd'))
	      {
		/* character literal */
		raw++;
		if (!xisdigit(raw[0]))
		    goto bug;
		dataLen = xstrtol_10(raw, &raw);
		if (raw[0] != '_')
		    goto bug;
		raw++;
		append_c(dest, '"');
		while (dataLen--)
		  {
		    if (xisxdigit(raw[0]) && xisxdigit(raw[1]))
			append_c(dest, (xasci2hex(raw[0]) << 4)
				+ xasci2hex(raw[1]));
		    else
			append_c(dest, '?');
		    raw += 2;
		  }
		append_c(dest, '"');
	      }
	    else
		goto bug;
	  }
	else if(raw[0] == 'Z')
	    /* end of parameter list */
	    break;
	else if(raw[0] == 'a')
	  {
	    append(dest, "char");
	    ++raw;
	  }
	else if(raw[0] == 'u')
	  {
	    append(dest, "wchar");
	    ++raw;
	  }
	else if(raw[0] == 'w')
	  {
	    append(dest, "dchar");
	    ++raw;
	  }
	else
	  {
bug:
	    append(dest, " @bug@[1]{");
	    append(dest, raw);
	    append_c(dest, '}');
	    break;
	  }
      }
    append_c(dest, ')');
}

/* Demangle the NULL-terminated D symbol and return the UTF-8 encoded
   representation or NULL.  The caller is responsible to free input and
   output.  */
char*
demangle_d(source)
	char* source;
{
    string_t dest;
    string_t nested;
    char* back = NULL;
    int mac_hack = 0;

    if ((source[0] == '_') && (source[1] == 'D') && (xisdigit(source[2]))
	    && (source[2] != '0'))
      {
	source += 2;
      }
#if defined(__APPLE__)
    // additional leading underscore on Mac systems:
    // http://d.puremagic.com/issues/show_bug.cgi?id=906
    else if ((source[0] == '_') && (source[1] == '_') && (source[2] == 'D')
	    && xisdigit(source[3]) && (source[3] != '0'))
      {
	source += 3;
      }
#endif
    else
	return NULL;

    dest = new_string();

    source = next_type(dest, source, 0);

    while (source && source[0])
      {
	/* nested symbols */
	nested = new_string();
	append_c(dest, '.');
	source = next_type(nested, source, 0);
	append_n(dest, nested->str, nested->used);
	xfree(nested->str);
	xfree(nested);
      }

    back = xstrndup(dest->str, dest->used+1);
    xfree(dest->str);
    xfree(dest);

    return back;
}
