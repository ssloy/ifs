#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#include <stdlib.h>

#include "automation.h"
#include "matrix.h"
#include "parser.h"

#define SIZE_X 1640
#define SIZE_Y 1640
#define STEPS  3




int PRIMITIVES_ARE_CLOSED = 1;
int NSTEPS = 0;
int VRML   = 1;
char cnt='a';

void iterate(automation_t &automation, fmatrix f, std::string state, int depth, std::ofstream &out) {
	if (depth+1<=NSTEPS) {
		int nedges = automation.nadjacent(state);
		for (int i=0; i<nedges; i++) {
			edge_t edge = automation.get_edge(state, i);
			fmatrix foo = automation.get_ftrans(edge.get_trans_name());
//			if (edge.get_trans_name().find("V")==std::string::npos && edge.get_trans_name().find("B")==std::string::npos) {
//			std::cout << state << "-" << edge.get_trans_name() << "\n";
			iterate(automation, f*foo, edge.get_target(), depth+1, out);
//			}
		}
	} else {
		int nprimitives = automation.nprimitives(state);
		for (int i=0; i<nprimitives; i++) {
			fmatrix primitives = automation.get_primitive(state, i);
			primitives = f*primitives;

			if (VRML) {
				if (primitives.ncols()<2) continue;
//				out << "Separator { Coordinate3 {\npoint [";
				out << "Shape { geometry IndexedFaceSet { coord Coordinate {\npoint [";
				for (int j=0; j<primitives.ncols(); j++)
					out << primitives[0][j] << " " << primitives[1][j] << " " << (primitives.nrows()>2 ? primitives[2][j] : 1) << ", ";
				if (PRIMITIVES_ARE_CLOSED)
					out << primitives[0][0] << " " << primitives[1][0] << " " << (primitives.nrows()>2 ? primitives[2][0] : 1) << ", ";
				out << "]}\n";

//				out << "ShapeHints { vertexOrdering CLOCKWISE shapeType SOLID}";
//				out << "Material { diffuseColor  1 0.2 0.2 }";
				int num = (PRIMITIVES_ARE_CLOSED ? primitives.ncols()+1 : primitives.ncols());
				out << "coordIndex[";
				for (int z=0; z<num; z++) {
					out << z<<" ";
				}
				out << "-1]\n";
//				out << "FaceSet{numVertices["<< (PRIMITIVES_ARE_CLOSED ? primitives.ncols()+1 : primitives.ncols()) << "]}\n";
//				out << "Material { diffuseColor  1 0.8 0 }";
//				out << "ShapeHints { vertexOrdering COUNTERCLOCKWISE shapeType SOLID}";
//				out << "FaceSet{numVertices["<< (PRIMITIVES_ARE_CLOSED ? primitives.ncols()+1 : primitives.ncols()) << "]}\n";
				out << "}}\n";
			} else {
				if (primitives.ncols()>1) {
					for (int j=0; j<primitives.ncols(); j++)
						out << (primitives[0][j]+1.0)/2.0*SIZE_X << " " << (1.0+primitives[1][j])/2.0*SIZE_Y << " " << ((0==j) ? "m" : "l") << " ";
					if (1 != primitives.ncols()) out << (PRIMITIVES_ARE_CLOSED ? " CL" : "" ) << " S\n";
				} else {
						out << (primitives[0][0]+10.0)/20.0*SIZE_X << " " << (10.0+primitives[1][0])/20.0*SIZE_Y << " circ \n ";
				}
			}
		}
	}
}

int main(int argc, char **argv) {
	if (2>argc) { std::cout << "Usage: " << argv[0] << " automation.txt" << std::endl; return (1); }
	if (argc==2 || !parseT(argv[2], NSTEPS)) {
		NSTEPS = STEPS;
	}

	automation_t automation = automation_t(argv[1]);

	std::cout << automation << std::endl;
	if (!automation.is_determined()) {
		std::cerr << "the automation contains non-defined variables" << std::endl;
		automation.fill_with_random_coefficients();
//		return (1);
	}
//	std::cout << automation << std::endl;


	std::stringstream buf;
	buf << time(NULL) << (VRML ? ".wrl" : ".ps");
	std::cerr << "filename: " << buf.str() << std::endl;

	std::ofstream out;
	out.open(buf.str().c_str());

	if (VRML) {
//		out << "#VRML V1.0 ascii\n";
		out << "#VRML V2.0 utf8\n";
//		out << "#Inventor V2.1 ascii\nSeparator { DrawStyle {pointSize 4 lineWidth 2 }\n";
//		out << "PerspectiveCamera { \n";
//		out << "}\n";
//			out << "ShapeHints { vertexOrdering CLOCKWISE shapeType SOLID}";
//				out << "Material { diffuseColor  1 0.2 0.2 }";
	} else {
		out << "%!PS-Adobe-3.0\n%%BoundingBox: 0 0 " << SIZE_X << " " << SIZE_Y << "\n%Pages: (atend)\n/circ{1 0 360 arc closepath fill}def /l{lineto}def /m{moveto}def /rm{rmoveto}def /rl{rlineto}def /S{stroke}def /CL{closepath}def /RGB{setrgbcolor}def /BLACK{0.1 0.1 0 RGB}def /GREEN{0 0 0 RGB}def /BLUE{0.174 1 1 RGB}def /RED{1 0 0 RGB}def\n\n%Page: 1\n0.1 setlinewidth\n";
		fmatrix primitives = automation.get_ftrans("P");
		for (int j=0; j<primitives.ncols(); j++) {
//			out << primitives[0][j]*SIZE_X << " " << primitives[1][j]*SIZE_Y << " circ\n";
		}
	}

	fmatrix f;
	f.identity(automation.get_primitive(automation.get_initial_state(), 0).nrows());
	iterate(automation,f, automation.get_initial_state(), 0, out);

	if (VRML) {
//		out << "}\n";
	}else
		out << "showpage\n%Trailer\n%Pages: 1\n";
	out.close();

	std::stringstream buf2;
	buf2 << (VRML ? "ivview " :"gv ") << buf.str();
	system(buf2.str().c_str());

	return 0;
}

