/*
    demangle_d - pluggable D de-mangler
    Copyright (C) 2006 Thomas Kuehne <thomas@kuehne.cn>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    As a special exception, the copyright holders of this library give you
    permission to link this library with independent modules to produce an
    executable, regardless of the license terms of these independent modules,
    and to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent module,
    the terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library. If
    you modify this library, you may extend this exception to your version of
    the library, but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* This file provieds functions for creating and modifing dynamic sized
    strings.  */

#include "config.h"
#include "string.h"
#include "util.h"

/* Create and initialize a new string.  */
string_t
new_string(void)
{
    string_t str = xmalloc(sizeof(string_t_r));
    str->used = 0;
    str->len = 128;
    str->str = xmalloc(str->len);
    str->str[0] = '\x00';
    return str;
}

/* Append len bytes from source string to dest string.  */
void
append_n(dest, source, len)
    string_t dest; const char* source; size_t len;
{
    size_t new_len;
    new_len = dest->used + len + 1;
    if (new_len > dest->len)
      {
	dest->len = new_len + (new_len >> 1);
	dest->str = xrealloc(dest->str, dest->len);
      }
    xmemcpy(dest->str + dest->used, source, len);
    dest->used += len;
    dest->str[dest->used] = '\x00';
}

/* Append source char to dest string.  */
void
append_c(dest, source)
    string_t dest; int source;
{
    size_t new_len;
    new_len = dest->used + 2;
    if (new_len > dest->len)
      {
	dest->len = new_len + (new_len >> 1);
	dest->str = xrealloc(dest->str, dest->len);
      }
    dest->str[dest->used++] = (char)source;
    dest->str[dest->used] = '\x00';
}

/* Append NULL-terminated string source to dest.  */
void
append(dest, source)
    string_t dest; const char* source;
{
    append_n(dest, source, xstrlen(source));
}

/* Prepend len bytes from source string to dest string.  */
void
prepend_n(dest, source, len)
    string_t dest; const char* source; size_t len;
{
    size_t new_len;
    new_len = dest->used + len + 1;
    if (new_len > dest->len)
      {
	dest->len = new_len + (new_len >> 1);
	dest->str = xrealloc(dest->str, dest->len);
      }

    if (dest->used)
	xmemmove(dest->str + len, dest->str, dest->used);

    xmemcpy(dest->str, source, len);
    dest->used += len;
    dest->str[dest->used] = '\x00';
}

/* Prepend NULL-terminated string source to dest.  */
void
prepend(dest, source)
    string_t dest; const char* source;
{
    prepend_n(dest, source, xstrlen(source));
}

/* If not nested, append len bytes from source string to dest else
   prepend with space.  */
void
nestpend_n(dest, source, len, is_nested)
    string_t dest; const char* source; size_t len; int is_nested;
{
    if (is_nested)
	append_n(dest, source, len);
    else
      {
	prepend(dest, " ");
	prepend_n(dest, source, len);
      }
}

/* If not nested, append NULL-terminated source string to dest else
   prepend with space.  */
void
nestpend(dest, source, is_nested)
    string_t dest; const char* source; int is_nested;
{
    nestpend_n(dest, source, xstrlen(source), is_nested);
}

