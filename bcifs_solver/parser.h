#ifndef __PARSER_H__
#define __PARSER_H__

#include <string.h>
#include "matrix.h"

/*
   Template parser library supported types:
     char  - any nonspace literal
     int   - +/- integer value (e.g -34 ),
     float = +/- float value (e.g -3.1415 ),
   Return value - pointer to element terminating symbol or zero if error
*/

typedef const char * cstr;

template <class t> cstr parseT(cstr, t&, bool comments=true);

template <> cstr parseT<char>(cstr str, char &v, bool cpp);
template <> cstr parseT<int>(cstr str, int &v, bool cpp);
template <> cstr parseT<float>(cstr str, float &v, bool cpp);
template <> cstr parseT<fmatrix>(cstr str, fmatrix &matrix, bool cpp);
template <> cstr parseT<smatrix>(cstr str, smatrix &matrix, bool cpp);

cstr get_line(cstr in, char *dest, int dest_size, bool skip_empty);
cstr skip_spaces(cstr s, bool comments);
fmatrix s2fmatrix(smatrix &s);

#endif //__PARSER_H__
