#ifndef __AUTOMATION_H__
#define __AUTOMATION_H__

#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "matrix.h"
#include "parser.h"

class edge_t {
public:
	edge_t(std::string source, std::string target, std::string trans_name);
	std::string get_source();
	std::string get_target();
	std::string get_trans_name();
private:
	std::string    _source;
	std::string    _target;
	std::string    _trans_name;
};

/*
substitute_t is very important structure, handles all the constraints.
for example: in the input file we have provided the following constraints:
a = 0
b = .5
c = a
d = e
e = f
we shall get a vector of vectors of strings of the following shape:

0	d	0.5
a	e	b
c	f

next, if we call the function substitute:

substitute("a") returns "0"
substitute("b") returns "0.5"
substitute("c") returns "0"
substitute("f") returns "d"

The fact that substitute ("c") returns "0" instead "a" is guaranteed
by the keeping the structure sorted (the function sort_data)
*/


class substitute_t {
public:
	substitute_t();
	void add_constraint(std::string a, std::string b);
	std::string substitute(std::string tosubst);
	friend std::ostream& operator<<(std::ostream& s, substitute_t& subst);
	void sort_data();
//private:
	std::vector<std::vector<std::string> > _data;
};

/*
The class of the automation.
NB: all the transformations are stored as matrices of strings, even if the input file
does not contain any free variable.

However, all the drawing shapes (primitives d'affichage) are stored as matrices of floats.
Thus, must not contain any variables.
*/
class automation_t {
public:
	automation_t(char *filename);
	std::string get_initial_state();
	int nadjacent(std::string state);
	edge_t get_edge(std::string state, int idx_adjacent);
	int nprimitives(std::string state);
	fmatrix get_primitive(std::string state, int idx);
	friend std::ostream& operator<<(std::ostream& s, automation_t& a);

	smatrix get_strans(std::string trans_name);
	fmatrix get_ftrans(std::string trans_name);
	bool is_determined();
	bool fill_with_random_coefficients();

private:
	std::string _initial_state;
	std::map<std::string, std::vector<edge_t>  > _adjacency_map;
	std::map<std::string, std::vector<fmatrix> > _primitives;
	std::map<std::string, smatrix > _strans;
	substitute_t _substitute;

	smatrix _multiply(cstr string);
	void _parse_automation_buffer(cstr buffer);
	void _parse_constraints_buffer(cstr line);
	void _include_automation(cstr line);
	void _perform_global_substitution();
};

#endif //__AUTOMATION_H__
