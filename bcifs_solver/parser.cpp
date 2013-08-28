#include "parser.h"

inline bool is_eol_char(int x)  { return '\n'==x || '\r'==x; }
inline bool is_eol     (int x)  { return is_eol_char(x) || x==0; }
inline bool is_space   (int x)  { return x==' ' || x=='\r' || x=='\n' || x=='\t'; }
inline cstr skip_eols  (cstr s) { while (is_eol_char(*s)) ++s; return s; }
inline bool is_digit   (int x)  { return x>='0' && x<='9'; }      // 0..9
inline int  int_value  (int x)  { return x-'0'; }                 // '5' -> 5

cstr get_line(cstr in, char *dest, int dest_size, bool skip_empty) {
	if (!in) return 0;
	if (skip_empty) in = skip_eols(in);
	if (dest) {  // copy string to dest
		for (; --dest_size && !is_eol(*in);  ++dest, ++in) *dest = *in;
		*dest = 0;
	} else // skip string
		while ( !is_eol(*in) ) ++in;
	if (skip_empty)
		in = skip_eols(in);
	else
		if ( is_eol_char(in[0]) )
			in += ( is_eol_char(in[1]) && in[1]!=in[0] ) ? 2 : 1;
	return in;
}

cstr skip_spaces(cstr s, bool comments) {
	for (;;) {
		while (is_space(*s)) s++;
		if (comments && (s[0]=='/' && s[1]=='/' || '#'==s[0]))
			s = get_line(s+('#'==s[0] ? 1 : 2),0,0,false);
		else break;
	}
	return s;
}

template <> cstr parseT<char>(cstr str, char &v, bool cpp) {
	str = skip_spaces( str, cpp );
	v   = *str;
	return str+1;
}

template <> cstr parseT<int>(cstr str, int &v, bool cpp) {
	str = skip_spaces( str, cpp );
	bool minus = (*str=='-') ? (++str,true) : false; // minus
	if (is_digit(*str)) {
		int value = 0;
		do
			value = value*10 + int_value(*str++);
		while ( is_digit(*str) );
		v =  minus ? -value : value;
	} else str = 0;
	return str;
}

template <> cstr parseT<float>(cstr str, float &v, bool cpp) {
	str = skip_spaces( str, cpp );
	bool minus = *str=='-' ? (++str, true) : false;
	float value=0;
	int   cnt=0; // valid symbol counter
	// get integer part
	for (; is_digit(*str); ++cnt, ++str)
		value = value*10 + int_value(*str);
	// get partial part
	if (*str=='.') {
		float pow = 1; ++str;
		for (; is_digit(*str); ++cnt, ++str) {
			pow/=10; value += pow * int_value(*str);
		}
	}
	if (!cnt) str=0;
	else v = minus ? -value : value;
	return str;
}

template <> cstr parseT<fmatrix>(cstr str, fmatrix &matrix, bool cpp) {
	matrix = fmatrix(0, 0);
	str = skip_spaces(str, cpp);
	int max_line_size = 1048576;
	char *line = new char[max_line_size];
	do {
		memset(line, '\0', max_line_size);
		str = get_line(str, line, max_line_size, false);
		std::vector<float> v;
		float f;
		const char *cur = line;
		while ((cur = parseT(cur, f)))
			v.push_back(f);
		if (v.size()>0)
			matrix.append_row(v);
	} while (str && strlen(line)>0);
	delete [] line;
	if (!matrix.nrows()) str=0;
	return str;
}

template <> cstr parseT<std::string>(cstr str, std::string &v, bool cpp) {
	str = skip_spaces( str, cpp );
	bool foundsmth = false;
	for (v=""; str && '\0'!=*str && !is_space(*str); v+=*str++) {
		foundsmth = true;
	}
	if (foundsmth) return str;
	v = "undef";
	return 0;
}

template <> cstr parseT<smatrix>(cstr str, smatrix &matrix, bool cpp) {
	matrix = smatrix(0, 0);
	str = skip_spaces(str, cpp);
	int max_line_size = 1048576;
	char *line = new char[max_line_size];
	do {
		memset(line, '\0', max_line_size);
		str = get_line(str, line, max_line_size, false);
		std::vector<string_element_t> v;
		std::string s;
		const char *cur = line;
		while ((cur = parseT(cur, s))) {
			v.push_back(string_element_t(s));
		}
		if (v.size()>0)
			matrix.append_row(v);
	} while (str && strlen(line)>0);
	delete [] line;
	if (!matrix.nrows()) str=0;
	return str;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// takes a string matrix and transforms it to a float matrix
fmatrix s2fmatrix(smatrix &s) {
	fmatrix result = fmatrix(s.nrows(), s.ncols());
	for (int i=0; i<s.nrows(); i++)
		for (int j=0; j<s.ncols(); j++)
			parseT(s[i][j].value().c_str(), result[i][j]);
	return result;
}

