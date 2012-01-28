#ifndef DEMANGLE_D_DEMANGLE_H
#define DEMANGLE_D_DEMANGLE_H 1

#define DD_(str) str

#ifdef __cplusplus
extern "C" {
#endif

/* demangle a D symbol
 *
 * input:
 * 	a NULL terminated mangled symbol
 *
 * output:
 * 	UTF-8 encoded demangled symbol
 * 	or NULL if unable to demangle
 *
 * memory:
 * 	the caller is responsible to
 * 	free input and output
 */
char* DD_(demangle_d)(char*);



/* return copyright message
 *
 * When linked in dynamically, this allows for the
 * copyright notice to be changed without having to
 * relink the main program.
 */
const char* demangle_d_copyright();

#ifdef __cplusplus
}
#endif

#endif /* DEMANGLE_D_DEMANGLE_H */
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
