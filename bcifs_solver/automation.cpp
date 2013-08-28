#include <iostream>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include "automation.h"


edge_t::edge_t(std::string source, std::string target, std::string trans_name) : _source(source), _target(target), _trans_name(trans_name) {
}

std::string edge_t::get_source() {
	return _source;
}

std::string edge_t::get_target() {
	return _target;
}

std::string edge_t::get_trans_name() {
	return _trans_name;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

substitute_t::substitute_t() {
}

std::string substitute_t::substitute(std::string tosubst) {
	for (unsigned int i=0; i<_data.size(); i++) {
		for (unsigned int j=0; j<_data[i].size(); j++) {
			if (_data[i][j]==tosubst) {
				return _data[i][0];
			}
		}
	}
	return tosubst;
}

void substitute_t::sort_data() {
	// just for fun let us sort the constraints
	for (unsigned int i=0; i<_data.size(); i++) {
		std::sort(_data[i].begin(), _data[i].end());
//		for (unsigned int j=0; j<_data[i].size(); j++) {
//			for (unsigned int k=j+1; k<_data[i].size(); k++) {
//				if (_data[i][k] < _data[i][j]) {
//					std::string z = _data[i][j];
//					_data[i][j] = _data[i][k];
//					_data[i][k] = z;
//				}
//			}
//		}

		// let us check if we have two different numbers in the same equalty list :)
		if (_data[i].size()>=2) {
			float a;
			float b;
			bool afloat = parseT(_data[i][0].c_str(), a);
			bool bfloat = parseT(_data[i][1].c_str(), b);
			if (afloat && bfloat) {
				if (a!=b) std::cerr << *this << std::endl;
				assert(a==b);
			}
		}
	}
}


// gets a proposition that "a" = "b"
// if nor a nor b are not found the structure, adds a new vector ("a", "b")
// otherwise there are two possible situations:
// 1. "a" exists, "b" does not - appends "b" to the vector containing "a" (same for the symmetric case)
// 2. more interesting: "a" and "b" exist in the structure in different vectors - merges the vectors
void substitute_t::add_constraint(std::string a, std::string b) {
	if (a==b) return; // nothing to add
	float foo;

	int aidx = -1;
	int bidx = -1;
	for (unsigned int i=0; (-1==aidx||-1==bidx) && i<_data.size(); i++) {
		for (unsigned int j=0; (-1==aidx||-1==bidx) && j<_data[i].size(); j++) {
			if (_data[i][j]==a) aidx = i;
			if (_data[i][j]==b) bidx = i;
		}
	}

	if (aidx==bidx && -1!=aidx) return; // processed
	if (-1==aidx && -1==bidx) {         // both do not exist
		std::vector<std::string> z;
		z.push_back(a);
		if (parseT(b.c_str(), foo))
			z.insert(z.begin(), b);
		else
			z.push_back(b);
		_data.push_back(z);
	} else if (-1==aidx) {              // have found b, but not a
		bool flag = true;
		for (unsigned int k=0; k<_data[bidx].size(); k++)
			if (_data[bidx][k]==a) flag = false;
		if (flag)
			_data[bidx].push_back(a);
	} else if (-1==bidx) {              // have found a, but not b
		bool flag = true;
		for (unsigned int k=0; k<_data[aidx].size(); k++)
			if (_data[aidx][k]==b) flag = false;
		if (flag)
			_data[aidx].push_back(b);
	} else {                            // both are found in different columns, let us merge the columns
		for (unsigned int j=0; j<_data[bidx].size(); j++) {
			bool flag = true;
			for (unsigned int k=0; k<_data[aidx].size(); k++)
				if (_data[aidx][k]==_data[bidx][j]) flag = false;
			if (flag)
				_data[aidx].push_back(_data[bidx][j]);
		}
		_data.erase(_data.begin()+bidx);
	}
}

std::ostream& operator<<(std::ostream& s, substitute_t& subst) {
	for (unsigned int i=0; i<subst._data.size(); i++) {
		for (unsigned int j=0; j<subst._data[i].size(); j++) {
			s << subst._data[i][j];
			if (j < subst._data[i].size()-1) s << " = ";
		}
		s << std::endl;
	}
	return s;
}

smatrix automation_t::_multiply(cstr string) {
	std::string s;
	smatrix cm;
	bool first = true;
//	std::cout << "in " << _initial_state << " " << string << "\n";
	while ((string = parseT(string, s))) {
		std::map<std::string, smatrix >::iterator iter = _strans.find(s);
		smatrix temp;
		if (iter==_strans.end()) {
			temp = smatrix(1,1);
			temp[0][0] = s;
		} else {
			temp = iter->second;
		}
		cm = first ? temp : cm*temp;
		first = false;
	}
	return cm;
}

#define HIERARCHICAL_INCLUDE 1

void automation_t::_include_automation(cstr line) {
//	if ("undef"==_initial_state) _initial_state = trans_name;
	char *b = strchr((char *)line, '<');
	char *e = strchr((char *)line, '>');
	assert (b && e && e>b);
	std::string state_name = std::string(line, b-line);
	std::string file_name(b+1, e-b-1);
//	std::cout << state_name << std::endl << file_name << std::endl;
	std::cerr << "including " << file_name << " from " << _initial_state << "\n";
	automation_t include = automation_t((char *)(file_name.c_str()));
	std::string inc_in_st = include.get_initial_state();
//	for (std::map<std::string, fmatrix >::iterator iter = include._primitives.begin(); iter!=include._primitives.end(); ++iter) {
	for (std::map<std::string, std::vector<fmatrix> >::iterator iter = include._primitives.begin(); iter!=include._primitives.end(); ++iter) {
#if HIERARCHICAL_INCLUDE
		std::string t_state_name = (inc_in_st==iter->first) ? state_name : state_name+"."+iter->first ;
#else
		std::string t_state_name = (inc_in_st==iter->first) ? state_name : iter->first ;
#endif
		_primitives[t_state_name] = iter->second;
	}
	for (std::map<std::string, smatrix >::iterator iter = include._strans.begin(); iter!=include._strans.end(); ++iter) {
#if HIERARCHICAL_INCLUDE
		std::string t_state_name = (inc_in_st==iter->first) ? state_name : state_name+"."+iter->first ;
#else
		std::string t_state_name = (inc_in_st==iter->first) ? state_name : iter->first ;
#endif
		smatrix c = iter->second;
		for (int i=0; i<c.nrows(); i++) {
			for (int j=0; j<c.ncols(); j++) {
				std::string z = c[i][j].value();
				if ('{'==z[0]) {
					//.TODO probably a bug with initial state
#if HIERARCHICAL_INCLUDE
					z.insert(1, state_name+".");
#endif
					c[i][j] = string_element_t(z);
				}
			}
		}
		//.TODO: {prefix
		_strans[t_state_name] = c;
	}
	// substitute!
	// .TODO {prefix will not work in all cases...
	for (unsigned int i=0; i<include._substitute._data.size(); i++) {
		for (unsigned int j=1; j<include._substitute._data[i].size(); j++) {
			std::string a = include._substitute._data[i][j-1];
			std::string b = include._substitute._data[i][j];
#if HIERARCHICAL_INCLUDE
			if ('{'==a[0] || '{'==b[0]) continue;
#endif
			/*
			if ('{'==a[0]) {
				a.insert(1, state_name+".");
			}
			if ('{'==b[0]) {
				b.insert(1, state_name+".");
			}
			*/

			_substitute.add_constraint(a, b);
		}
	}
	for (std::map<std::string, std::vector<edge_t> >::iterator iter = include._adjacency_map.begin(); iter!=include._adjacency_map.end(); ++iter){
		int size = iter->second.size();
		for (int i=0; i<size; i++) {
			edge_t edge = iter->second[i];
			std::string source = edge.get_source();
			std::string target = edge.get_target();
#if HIERARCHICAL_INCLUDE
			source = (source==inc_in_st) ? state_name : state_name+"."+source;
			target = (target==inc_in_st) ? state_name : state_name+"."+target;
			std::string trans_name = state_name+"."+edge.get_trans_name();
			_adjacency_map[source].push_back(edge_t(source, target, trans_name));
#else
			source = (source==inc_in_st) ? state_name : source;
			target = (target==inc_in_st) ? state_name : target;
			std::string trans_name = edge.get_trans_name();
			bool ok = true;
			for (int z=0; ok && z<nadjacent(source); z++) {
				edge_t zedge = get_edge(source, z);
				if (zedge.get_trans_name()==trans_name) ok = false;
			}
			if (ok) _adjacency_map[source].push_back(edge_t(source, target, trans_name));
#endif
//			smatrix z = a.get_strans(edge.get_trans_name());
//			s << edge.get_trans_name() << ":" << iter->first << "-" << edge.get_target() << std::endl << z << std::endl;
		}
	}
//	std::cout << include << std::endl;
//	std::cout << *this << std::endl;
}

void automation_t::_parse_constraints_buffer(cstr line) {
	char *equal_mark = strchr((char *)line, '=');
	if (!equal_mark) return;
	char *left = new char[equal_mark-line+1];
	strncpy(left, line, equal_mark-line);
//	std::cout << "initial state:" << _initial_state << "\n";
//	std::cout << "line:" << line << "\n";
	left[equal_mark-line] = '\0';
	smatrix cm  = _multiply(left);
	smatrix cm2 = _multiply(equal_mark+1);
//	std::cout << cm << std::endl << std::endl << cm2 << std::endl;
	if (cm.nrows()!=cm2.nrows() || cm.ncols()!=cm2.ncols()) {
		std::cout << "===========================\n";
		std::cout << line << std::endl << std::endl << cm << std::endl << cm2;
		std::cout << "===========================\n";
	}
	assert(cm.nrows()==cm2.nrows() && cm.ncols()==cm2.ncols());

//	std::cout << "===========================\n";
//	std::cout << line << std::endl << std::endl << cm << std::endl << cm2;

	for (int i=0; i<cm.nrows(); i++) {
		for (int j=0; j<cm.ncols(); j++) {
			assert(CANT_PROCESS!=cm[i][j].value() && CANT_PROCESS!=cm2[i][j].value());
			_substitute.add_constraint(cm[i][j].value(), cm2[i][j].value());
		}
	}
//	std::cout << "===========================\n";
	delete [] left;
}

// reads the input buffer line by line, if necessary,
// includes sub-automations, adds constrains etc.
void automation_t::_parse_automation_buffer(cstr buffer) {
	int max_line_size = 1048576;
	char *line = new char[max_line_size];

//std::cout << "=====================================\n" << buffer << "\n=========================\n";
	while (buffer && strlen(buffer)>0) {
		memset(line, '\0', max_line_size);
		buffer = skip_spaces(buffer, true);
		buffer = get_line(buffer, line, max_line_size, false);

		if (strchr(line, '=')) { // it is a constraint line
//			std::cout << "~~\n" << line << "\n~~\n";
			_parse_constraints_buffer(line);
			continue;
		}

		if (strchr(line, '<') && strchr(line, '>')) { // it is an automation include line
			_include_automation(line);
			continue;
		}
		char *comma = strchr(line, ':');
		if (!comma) continue; // not a valid transformation/primitives description
		std::string trans_name = std::string(line, comma-line);
		if ("undef"==_initial_state) _initial_state = trans_name;
		char *dash  = strchr(line, '-');
		if (!dash) { // just some primitives to draw
			fmatrix foo;
//			buffer = parseT(buffer, _primitives[trans_name], true);
			buffer = parseT(buffer, foo, true);
			_primitives[trans_name].push_back(foo);
			continue;
		}

		// ok. we have a transformation
		smatrix curmatrix;
		buffer = parseT(buffer, curmatrix, true);
		if (!buffer) continue; // if there is a matrix

		std::string source(comma+1, dash-comma-1);
		std::string target(dash+1);
		if (1==curmatrix.nrows() && 1==curmatrix.ncols() && curmatrix[0][0]=="?") { // auto-fill the smatrix
//			curmatrix = smatrix(_primitives[source].nrows(), _primitives[target].nrows());
			//.TODO check for existence [0]
			curmatrix = smatrix(_primitives[source][0].nrows(), _primitives[target][0].nrows());
			for (int i=0; i<curmatrix.nrows(); i++) {
				for (int j=0; j<curmatrix.ncols(); j++) {
					curmatrix[i][j] = string_element_t("{"+trans_name + "_" +(char)(j+'A')+(char)(i+'A')+"}");
				}
			}
		}
		_adjacency_map[source].push_back(edge_t(source, target, trans_name));
		_strans[trans_name] = curmatrix;
	}
	delete [] line;
}

// reads the input file,
// tries to solve the constrains and to substitute unused variables
automation_t::automation_t(char *filename) : _initial_state("undef") {
	std::ifstream in;
	in.open (filename, std::ifstream::in);
	if (in.fail()) return;
	in.seekg (0, std::ios::end);
	long length = in.tellg();
	in.seekg (0, std::ios::beg);
	char *buffer = new char [length+1];
	in.read (buffer,length);
	in.close();
	buffer[length] = '\0';

//	std::cout << "opening file " << filename << "\n";
	_parse_automation_buffer(buffer);
	delete [] buffer;
//	std::cout << _substitute << std::endl;
//	std::cout << *this << std::endl;
	_perform_global_substitution();
}

void automation_t::_perform_global_substitution() {
	std::cerr << "Performing global substitution" << std::endl;
	_substitute.sort_data();
	for (std::map<std::string, smatrix >::iterator iter = _strans.begin(); iter!=_strans.end(); ++iter) {
		for (int i=0; i<iter->second.nrows(); i++)
			for (int j=0; j<iter->second.ncols(); j++)
				iter->second[i][j] = _substitute.substitute(iter->second[i][j].value());
	}
//	_substitute = substitute_t();
}


std::string automation_t::get_initial_state() {
	return _initial_state;
}

int automation_t::nadjacent(std::string state) {
	return _adjacency_map[state].size();
}

int automation_t::nprimitives(std::string state) {
	return _primitives[state].size();
}

edge_t automation_t::get_edge(std::string state, int idx_adjacent) {
	return _adjacency_map[state][idx_adjacent];
}

fmatrix automation_t::get_primitive(std::string state, int idx) {
	return _primitives[state][idx];
}

smatrix automation_t::get_strans(std::string trans_name) {
	return _strans[trans_name];
}

fmatrix automation_t::get_ftrans(std::string trans_name) {
	return s2fmatrix(_strans[trans_name]);
}

// IMPORTANT!
// The output is crappy:
// 1. the initial state is not printed first!
// 2. in case of includes does not keep multiply files, the output is merged

std::ostream& operator<<(std::ostream& s, automation_t& a) {
	for (std::map<std::string, std::vector<fmatrix> >::iterator iter = a._primitives.begin(); iter!=a._primitives.end(); ++iter) {
		int size = iter->second.size();
		for (int i=0; i<size; i++) {
			if (iter->first != a._initial_state /*|| iter->first.find_first_of("fz")==std::string::npos*/) continue;
			s << iter->first << ":" << std::endl << iter->second[i] << std::endl;
		}
	}
	for (std::map<std::string, std::vector<fmatrix> >::iterator iter = a._primitives.begin(); iter!=a._primitives.end(); ++iter) {
		int size = iter->second.size();
		for (int i=0; i<size; i++) {
			if (iter->first == a._initial_state /*|| iter->first.find_first_of("fz")==std::string::npos*/) continue;
			s << iter->first << ":" << std::endl << iter->second[i] << std::endl;
		}
	}
	for (std::map<std::string, std::vector<edge_t> >::iterator iter = a._adjacency_map.begin(); iter!=a._adjacency_map.end(); ++iter){
		int size = iter->second.size();
		for (int i=0; i<size; i++) {
			edge_t edge = iter->second[i];
			smatrix z = a.get_strans(edge.get_trans_name());
//			if (edge.get_trans_name().find("f")==std::string::npos&&edge.get_trans_name().find("reverse")==std::string::npos) continue;
//			if (edge.get_trans_name().find("Tf")==std::string::npos&&edge.get_trans_name().find("P")==std::string::npos) continue;
			s << edge.get_trans_name() << ":" << iter->first << "-" << edge.get_target() << std::endl << z << std::endl;
		}
	}
//	s << a._substitute << std::endl;
	return s;
}

// are all variables gone or not?
bool automation_t::is_determined() {
	float foo;
	for (std::map<std::string, std::vector<edge_t> >::iterator iter = _adjacency_map.begin(); iter!=_adjacency_map.end(); ++iter){
		int size = iter->second.size();
		for (int i=0; i<size; i++) {
			edge_t edge = iter->second[i];
			smatrix z = get_strans(edge.get_trans_name());
			for (int i=0; i<z.nrows(); i++) {
				for (int j=0; j<z.ncols(); j++) {
					if (!parseT(z[i][j].value().c_str(), foo))
						return false;
				}
			}
		}
	}
	return true;
}

float sum_up_all_floats_in_a_string_column(smatrix &z, int col) {
	float result = 0;
	float foo = 0;
	for (int i=0; i<z.nrows(); i++) {
		if (parseT(z[i][col].value().c_str(), foo)) {
			result += foo;
		}
	}
	return result;
}

bool automation_t::fill_with_random_coefficients() {
	float foo;
	for (std::map<std::string, std::vector<edge_t> >::iterator iter = _adjacency_map.begin(); iter!=_adjacency_map.end(); ++iter) {
		int size = iter->second.size();
		for (int i=0; i<size; i++) {
			edge_t edge = iter->second[i];
			smatrix z = get_strans(edge.get_trans_name());
			for (int j=0; j<z.ncols(); j++) {
				float cur_sum = 0;
				//sum_up_all_floats_in_a_string_column(z, j);

				std::map<std::string, int> tied_vars;
				for (int i=0; i<z.nrows(); i++) {
					if (!parseT(z[i][j].value().c_str(), foo)) {
						std::map<std::string, int>::iterator el = tied_vars.find(z[i][j].value());
						if (el!=tied_vars.end()) {
							(el->second)++;
						} else {
							tied_vars[z[i][j].value()] = 1;
						}
					} else {
						cur_sum += foo;
					}
				}
				for (std::map<std::string, int>::iterator var = tied_vars.begin(); var!=tied_vars.end(); ++var) {
					std::map<std::string, int>::iterator var2 = var;
					float rnd = (++var2==tied_vars.end()) ? 1-cur_sum: (float)rand()/(float)RAND_MAX*(1-cur_sum);
					float coeff = rnd/var->second;
					std::stringstream buf;
					buf.setf(std::ios::fixed, std::ios::floatfield);
					std::cerr.setf(std::ios::fixed, std::ios::floatfield);
					buf << coeff;
					_substitute.add_constraint(var->first, buf.str());
					std::cerr << var->first << " = " << coeff << "\n";
					cur_sum += rnd;
				}
				if (tied_vars.size()) {
					_perform_global_substitution();
					fill_with_random_coefficients();
					return true;
				}
			}
		}
	}
	return true;
}
