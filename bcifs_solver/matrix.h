#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <iostream>
#include <vector>
#include <assert.h>

#define DEFAULT_ALLOC 2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class t> class matrix {
	std::vector<std::vector<t> > m;
	int rows, cols;
public:
	matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC);
	matrix(const matrix<t> &s);
	~matrix<t>();
	inline int nrows();
	inline int ncols();

	int append_row(std::vector<t> v);
	int append_col(std::vector<t> v);
	matrix<t>& operator =(const matrix<t>& s);
	matrix<t>& identity(int dimensions);
	inline std::vector<t>& operator[](const int i);
	template <class > friend matrix<t> operator*(const t &x, const matrix<t> &s);
	template <class > friend matrix<t> operator*(const matrix<t> &s, const t &x);
	matrix<t> operator*(const matrix<t>& a);
	matrix<t> operator+(const matrix<t>& a);
	matrix<t> operator-(const matrix<t>& a);
	matrix<t> transpose();
	matrix<t> inverse();

	template <class > friend std::ostream& operator<<(std::ostream& s, matrix<t>& m);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class t> matrix<t>::matrix(int r, int c) : rows(r), cols(c) {
	m.resize(rows);
	for (int i=0; i<rows; i++) {
		m[i].resize(cols);
	}
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			m[i][j] = t();
		}
	}
}

template <class t> matrix<t>::matrix(const matrix<t> &s) : rows(s.rows), cols(s.cols) {
	m = std::vector<std::vector<t> >(rows);
	for(int i=0;i<rows;i++) {
		m[i] = s.m[i];
	}
}

template <class t> matrix<t>::~matrix() {
}

template <class t> int matrix<t>::nrows() {
	return rows;
}

template <class t> int matrix<t>::ncols() {
	return cols;
}

template <class t> int matrix<t>::append_row(std::vector<t> v) {
	if (rows>0) {
		if (cols!=(int)v.size()) {
			std::cerr << cols << "!=" << (int)v.size() << std::endl << "xxxxxxxxxxxx\n" ;
			std::cerr << *this << std::endl << "xxxxxxxxx \n";
			for (int i=0; i<(int)v.size(); i++)
				std::cerr << v[i] << " ";
			std::cerr << std::endl;
		}
		assert(cols==(int)v.size());
	} else
		cols = v.size();
	m.push_back(v);
	return ++rows;
}

template <class t> int matrix<t>::append_col(std::vector<t> v) {
	if (cols>0) {
		assert(rows==(int)v.size());
	} else {
		rows = v.size();
		m.resize(rows);
	}
	for (int i=0; i<rows; i++) {
		m[i].push_back(v[i]);
	}
	return ++cols;
}

template <class t> matrix<t>& matrix<t>::operator =(const matrix<t>& s) {
	if(this != &s) {
		rows = s.rows;
		cols = s.cols;
		m = std::vector<std::vector<t> >(rows);
		for(int i=0;i<rows;i++) {
			m[i] = s.m[i];
		}
	}
	return *this;
}

template <class t> matrix<t>& matrix<t>::identity(int dimensions) {
	cols = rows = dimensions;
	m = std::vector<std::vector<t> >(rows);
	for(int i=0;i<rows;i++) {
		m[i].resize(cols);
	}
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			m[i][j] = (i==j ? t(1) : t(0));
		}
	}
	return *this;
}

template <class t> std::vector<t>& matrix<t>::operator[](const int i) {
	assert(i>=0 && i<rows);
	return m[i];
}

template <class t> matrix<t> operator*(const t &x, const matrix<t> &s) {
	matrix<t> result(s.rows,s.cols);
	for (int i=0; i<result.rows; i++) {
		result.m[i] = x*s.m[i];
	}
	return result;
}

template <class t> matrix<t> operator*(const matrix<t> &s, const t &x) {
	matrix<t> result(s.rows,s.cols);
	for (int i=0; i<result.rows; i++) {
		result.m[i] = x*s.m[i];
	}
	return result;
}

template <class t> matrix<t> matrix<t>::operator*(const matrix<t>& a) {
	if (cols!=a.rows) {
		matrix<t> z = a;
		std::cout << cols << "!=" << a.rows << "\n" << *this << "\n===========\n" << z << "\n";
	}
	assert(cols == a.rows);
	matrix<t> result(rows, a.cols);
	for (int i=0; i<rows; i++) {
		for (int j=0; j<a.cols; j++) {
			result.m[i][j] = t();
			for (int k=0; k<cols; k++) {
				result.m[i][j] += m[i][k]*a.m[k][j];
			}
		}
	}
	return result;
}

template <class t> matrix<t> matrix<t>::operator+(const matrix<t>& a) {
	assert(rows== a.rows || cols==a.cols);
	matrix<t> result(a.rows, a.cols);
	for (int i=0; i<a.rows; i++) {
		for (int j=0; j<a.cols; j++) {
			result.m[i][j] = m[i][j] + a.m[i][j];
		}
	}
	return result;
}


template <class t> matrix<t> matrix<t>::operator-(const matrix<t>& a) {
	assert(rows== a.rows || cols==a.cols);
	matrix<t> result(a.rows, a.cols);
	for (int i=0; i<a.rows; i++) {
		for (int j=0; j<a.cols; j++) {
			result.m[i][j] = m[i][j] - a.m[i][j];
		}
	}
	return result;
}


template <class t> matrix<t> matrix<t>::transpose() {
	matrix<t> result(cols, rows);
	for(int i=0; i<rows; i++)
		for(int j=0; j<cols; j++)
			result[j][i] = m[i][j];
	return result;
}

template <class t> matrix<t> matrix<t>::inverse() {
	assert(rows==cols);
	// augmenting the square matrix with the identity matrix of the same dimensions A => [AI]
	matrix<t> result(rows, cols*2);
	for(int i=0; i<rows; i++)
		for(int j=0; j<cols; j++)
			result[i][j] = m[i][j];
	for(int i=0; i<rows; i++)
		result[i][i+cols] = 1;
	// first pass
	for (int i=0; i<rows-1; i++) {
		// normalize the first row
		for(int j=result.cols-1; j>=0; j--)
			result[i][j] /= result[i][i];
		for (int k=i+1; k<rows; k++) {
			t coeff = result[k][i];
			for (int j=0; j<result.cols; j++) {
				result[k][j] -= result[i][j]*coeff;
			}
		}
	}
	// normalize the last row
	for(int j=result.cols-1; j>=rows-1; j--)
		result[rows-1][j] /= result[rows-1][rows-1];
	// second pass
	for (int i=rows-1; i>0; i--) {
		for (int k=i-1; k>=0; k--) {
			t coeff = result[k][i];
			for (int j=0; j<result.cols; j++) {
				result[k][j] -= result[i][j]*coeff;
			}
		}
	}
	// cut the identity matrix back
	matrix<t> truncate(rows, cols);
	for(int i=0; i<rows; i++)
		for(int j=0; j<cols; j++)
			truncate[i][j] = result[i][j+cols];
	return truncate;
}

template <class t> std::ostream& operator<<(std::ostream& s, matrix<t>& m) {
	for (int i=0; i<m.nrows(); i++)  {
		for (int j=0; j<m.ncols(); j++) {
			s << m[i][j];
			if (j<m.ncols()-1) s << "\t";
		}
		s << "\n";
	}
	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/*
in order to multiply string matrices, we have to introduce this class.
It handles operations between string matrices, e.g.

|a b| |0 1| gives |b a|
|c d| |1 0|       |d c|

However, it is quite limited and cannot multiply arbitrary string matrices:
|a|*|b| will give |CANT_PROCESS|,
since we know how to multiply a string by 1 and 0 only
*/

#define CANT_PROCESS ":("

class string_element_t {
public:
	string_element_t(std::string arg_value="0") {
		_value = arg_value;
	}
	inline std::string value() {
		return _value;
	}
	friend std::ostream& operator<<(std::ostream& s, string_element_t& str) {
		s << str._value;
		return s;
	}
	inline string_element_t operator*(const string_element_t& a) {
		if (_value==CANT_PROCESS || a._value==CANT_PROCESS) return string_element_t(CANT_PROCESS);
		if ("1"==_value)   return string_element_t(a._value);
		if ("0"==_value)   return string_element_t("0");
		if ("1"==a._value) return string_element_t(_value);
		if ("0"==a._value) return string_element_t("0");
		return string_element_t(CANT_PROCESS);
	}

	inline const string_element_t& operator += (const string_element_t &v) {
		if (_value=="0") _value = v._value;
		else if (v._value!="0") _value = CANT_PROCESS;
		return *this;
	}
	inline string_element_t operator+(const string_element_t& a) {
		if (_value==CANT_PROCESS || a._value==CANT_PROCESS) return string_element_t(CANT_PROCESS);
		if (a._value=="0") return string_element_t(_value);
		if (_value  =="0") return string_element_t(a._value);
		return string_element_t(CANT_PROCESS);
	}
	inline bool operator == (const string_element_t &v) const {
		return _value==v._value;
	}
	inline bool operator == (const std::string &v) const {
		return _value==v;
	}
private:
	std::string _value;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef matrix<float>  fmatrix;
typedef matrix<string_element_t>  smatrix;

#endif //__MATRIX_H__
