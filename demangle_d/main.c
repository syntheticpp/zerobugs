/*
 * demangle_d - pluggable D de-mangler
 * Copyright (C) 2006 Thomas Kuehne <thomas@kuehne.cn>
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

#ifdef DEMANGLE_D_STANDALONE
#include "parser.h"
#include "util.h"

int
main(int argc, char** argv)
{
   int i;
   if (argc < 2)
     {
      xfprintf(stderr,
         _("pluggable D d-demangler by Thomas Kuehne <thomas@kuehne.cn> (%s)\n"),
         "$Date: 2006-04-22T22:40:29.553250Z $");
      xfprintf(stderr, _("%s <mangledSymbol_1> [<mangledSymbol_2> ...]\n"),
	    argc ? argv[0] :"demangle_d");
      return (EXIT_FAILURE);
     }
   for (i = 1; i < argc; i++)
     {
      char* demangled = DD_(demangle_d)(argv[i]);
      if (1 > xprintf(_("%s\t%s\n"), argv[i], demangled))
         xperror(NULL);
      if (demangled)
         xfree(demangled);
     }
   return (EXIT_SUCCESS);
}
#endif /* DEMANGLE_D_STANDALONE */
