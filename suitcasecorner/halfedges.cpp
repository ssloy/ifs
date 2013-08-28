#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoCoordinate3.h>

#include <assert.h>
#include <iostream>
#include <list>
#include <vector>
#include <math.h>

#include "halfedges.h"

HE_edge::HE_edge(int _vert, HE_face *_face) : vert(_vert), pair(NULL), face(_face), next(NULL), prev(NULL), mark(false) {
}

HE_edge::HE_edge() : vert(-1),/* orig(-1),*/ pair(NULL), face(NULL), next(NULL), prev(NULL), mark(false) {
}

HE_face::HE_face() : edge(NULL), size(0) {
}

Mesh::Mesh() : edges(), faces(), verts() {
}

Mesh::~Mesh() {
	for (vector<HE_face *>::iterator p=faces.begin(); p != faces.end(); p++) {
		delete (HE_face *)(*p);
	}
	for (vector<HE_edge *>::iterator p=edges.begin(); p != edges.end(); p++) {
		delete (HE_edge *)(*p);
	}
}

void Mesh::clear_edge_marks() {
	for (vector<HE_edge *>::iterator p=edges.begin(); p != edges.end(); p++) {
		HE_edge *edge = *p;
		edge->mark = false;
	}
}

bool Mesh::doo_sabin(vector<vector<float> > coeffs) {
	clear_edge_marks();
	vector<float> verts2;
	for (vector<HE_edge *>::iterator p=edges.begin(); p!=edges.end(); p++) {
		HE_edge *edge = *p;
		assert(edge && edge->face);
		int n = edge->face->size;
		HE_edge *cur_edge = edge;
		int i = 0;
		float point[3] = {0,0,0};
		do {
//			assert(cur_edge && cur_edge->next && cur_edge->prev);
			float f = (cur_edge==edge) ? (n+5.0)/(4.0*n): (3+2*cos(2*M_PI*i/((float)n)))/(4.0*n);
//			if (n-2<=(int)coeffs.size()) {
			if (n==3||n==4) {
				f = coeffs[n-3][i];
			}
			for (int k=0; k<3; k++) {
				point[k] += f*verts[cur_edge->vert*3+k];
			}
			assert(cur_edge->next);
			cur_edge = cur_edge->next;
			i++;
		} while (cur_edge!=edge);
		for (int k=0; k<3; k++) {
			verts2.push_back(point[k]);
		}
	}
	int i = 0;
	for (vector<HE_edge *>::iterator p=edges.begin(); p!=edges.end(); p++) {
		HE_edge *edge = *p;
		edge->vert = i++;
	}
	verts = verts2;

	vector<HE_edge *> newedges;
	for (vector<HE_edge *>::iterator p=edges.begin(); p!=edges.end(); p++) {
		HE_edge *edge = *p;
		assert(edge);
		if (edge->mark || !edge->pair || !edge->pair->face) continue;
		assert(edge->prev && edge->next && edge->pair && edge->pair->prev && edge->pair->next);
		edge->mark = true;
		edge->pair->mark = true;
		HE_face *face = new HE_face();
		face->size = 4;
		faces.push_back(face);
		HE_edge *fe[] = {NULL, NULL, NULL, NULL};
		for (int i=0; i<4; i++) {
			fe[i] = new HE_edge();
			newedges.push_back(fe[i]);
		}
		for (int i=0; i<4; i++) {
			fe[i]->next = fe[(i+1  )%4];
			fe[i]->prev = fe[(i-1+4)%4];
			fe[i]->face = face;
		}
		face->edge = fe[0];
		fe[0]->pair = edge;
		fe[2]->pair = edge->pair;
		edge->pair->pair = fe[2];
		edge->pair = fe[0];
		fe[0]->vert = edge->prev->vert;
		fe[3]->vert = edge->vert;
		fe[1]->vert = fe[2]->pair->vert;
		fe[2]->vert = fe[2]->pair->prev->vert;
	}
	vector<HE_edge *> newedges2;
	for (vector<HE_edge *>::iterator p=newedges.begin(); p!=newedges.end(); p++) {
		HE_edge *edge = *p;
		assert(edge && edge->next && edge->prev);
		assert(edge!=edge->pair && edge!=edge->next && edge!=edge->prev);
		if (edge->pair || edge->mark) continue;

		HE_face *face = new HE_face();
		HE_edge *cur_edge = edge;
//		size_t nedges2 = newedges2.size();
		faces.push_back(face);
		do {
			cur_edge->mark = true;
			HE_edge *newedge = new HE_edge(cur_edge->prev->vert, face);
			newedges2.push_back(newedge);
			newedge->pair = cur_edge;
			if (!face->edge) {
				face->edge = newedge;
			}
			newedge->next = face->edge->next;
			face->edge->next = newedge;
			newedge->next->prev = newedge;
			newedge->prev = face->edge;
			face->size++;
			// TODO : поверхность может быть и не замкнутой...
			if (!cur_edge->next || !cur_edge->next->pair || !cur_edge->next->pair->next || !cur_edge->next->pair->next->pair || !cur_edge->next->pair->next->pair->next)  {
				for (int i=0; i<face->size; i++) {
					delete *(newedges2.end()-1);
					newedges2.pop_back();
				}
				delete face;
				face = NULL;
				faces.pop_back();
				break;
			}
			cur_edge = cur_edge->next->pair->next->pair->next;
		} while (cur_edge!=edge);
		for (int i=0; face && i<face->size; i++) {
			HE_edge *p = *(newedges2.end()-1-i);
			assert(p->pair);
			p->pair->pair = p;
		}
	}
	copy(newedges.begin(),  newedges.end(),  back_inserter(edges));
	copy(newedges2.begin(), newedges2.end(), back_inserter(edges));
	clear_edge_marks();
	return true;
}

std::ostream& operator<<(std::ostream& s, Mesh& he) {
	for (int i=0; i<(int)he.verts.size(); i++) {
		s << (i%3 ? " " : "\nv ") << he.verts[i];
	}
	s << "\n";

	for (vector<HE_face *>::iterator p=he.faces.begin(); p!=he.faces.end(); p++) {
		HE_face *face = *p;
		HE_edge *edge = face->edge;
		s << "f ";
		do {
			s << (edge->vert+1) << " ";
			edge = edge->next;
		} while (edge != face->edge);
		s << "\n";
	}

/*
	cerr << "Faces: " << he.faces.size() << "\n";
	// loop over faces
	for (vector<HE_face *>::iterator p=he.faces.begin(); p!=he.faces.end(); p++) {
		// loop over edges
		HE_face *face = *p;
		HE_edge *edge = face->edge;
		cerr << "Face: " << face->edge << "\n";
		do {
			cerr << "Edge: " << edge << " "
				<< edge->vert << " ("
				<< he.verts[edge->vert*3  ] << ", "
				<< he.verts[edge->vert*3+1] << ", "
				<< he.verts[edge->vert*3+2] << ") "
				<< edge->face << " "
				<< edge->pair << " "
				<< "prev: " << edge->prev << " "
				<< "next: " << edge->next << "\n";
			edge = edge->next;
		} while (edge != face->edge);
	}
*/
	return s;
}

// Loads a single faceset into the data structure.
// The strategy is to loop over faces, inserting half edges
// for each edge, which are linked together, including a link to a new face.
// For convenience, this new face is itself stored in a member list.
// Additionally, the half edges are stored in lists
// at each vertex. Then these new half edges are looped over again
// to compare with the lists at each vertex to identify half-edge pairs.
// Identified pairs are then removed from the vertex's edge list.
// In this way each half edge is linked with it's next half edge,
// a face, a pair, and of course a single vertex.
bool loadShape(SoIndexedFaceSet *shape, SoCoordinate3 *coords, Mesh &he) {
	for (vector<HE_face *>::iterator p=he.faces.begin(); p != he.faces.end(); p++) {
		delete (HE_face *)(*p);
	}
	for (vector<HE_edge *>::iterator p=he.edges.begin(); p != he.edges.end(); p++) {
		delete (HE_edge *)(*p);
	}
	he.edges = vector<HE_edge *>();
	he.faces = vector<HE_face *>();
	if (!shape || !coords || !shape->isOfType(SoIndexedFaceSet::getClassTypeId()) || !coords->isOfType(SoCoordinate3::getClassTypeId())) return false;

    int num_verts = coords->point.getNum();
	he.verts = vector<float>(num_verts*3);
	for (int i=0; i<num_verts; i++) {
		for (int j=0; j<3; j++) {
			he.verts[i*3+j] = coords->point[i][j];
		}
	}

    int num_idx   = shape->coordIndex.getNum();
	vector<list<HE_edge *> > vert_edges(num_verts);

	for (int j=0; j<num_idx; j++) { // loop over polygons, here j++ means skip -1 index
		HE_face *face = new HE_face();
		he.faces.push_back(face);
		HE_edge *prev_edge=NULL;
		for (; shape->coordIndex[j]>=0; j++) { // loop over vertices in the current polygon
			HE_edge *edge = new HE_edge(shape->coordIndex[j], face);
			he.edges.push_back(edge);
			face->size++;
			if (!face->edge) {
				face->edge = edge;
			} else {
				assert(prev_edge);
				prev_edge->next = edge;
//				edge->orig = prev_edge->vert; // for debugging purposes
			}
			edge->prev = prev_edge;
			prev_edge  = edge;
			// will be rewritten each time until the last edge
//			face->edge->orig = edge->vert;
			edge->next = face->edge;
			face->edge->prev = edge;
		}
		if (!face->edge) continue;
		HE_edge *edge = face->edge;
		do {
			assert(edge->next);
			int a = edge->vert;
			int b = edge->next->vert;
			// search b's edge list for edge going to a
			// assign half edge as pair
			list<HE_edge *>::iterator p = vert_edges[b].begin();
			for (; p != vert_edges[b].end(); p++) {
				HE_edge *edge2 = *p;
				assert(edge2);
				// found edge?
				if (edge2->vert!=a) continue;
				// link edges
				edge->next->pair = edge2;
				edge2->pair = edge->next;
				// remove edge and stop
				vert_edges[b].erase(p);
				break;
			}
			// if not found and not paired, add edge to the vert's list
			if (vert_edges[b].end()==p && !edge->next->pair) {
				vert_edges[a].push_back(edge->next);
			}
			edge = edge->next;
		} while (edge!=face->edge);
	}
	/*
    // check for unfound half-edge's
    int unpairedEdges = 0;
    int firstTime = 1;
	for (int i=0; i<num_verts; i++) {
		unpairedEdges += vert_edges[i].size();
		if (unpairedEdges && firstTime){
			cerr << "Unpaired vertices: \n";
			firstTime = 0;
		}
		if (vert_edges[i].size()) {
			cerr << "Vert " << i << ": ";
			// loop over unapred edges
			for (list <HE_edge *>::iterator p=vert_edges[i].begin(); p!=vert_edges[i].end(); ++p) {
				HE_edge *edge = *p;
				cerr << edge->vert << " ";
			}
			cerr << "\n";
		}
	}

    // Euler's formula: F+V-E=2
    // E.g., a cube:    6+8-12=2
    cerr << he.faces.size() << " faces, " << num_verts << " verts, " << he.edges.size() << " half edges (" << he.edges.size()/2 << " edges).\n";
    cerr << "Euler characteristic = " << he.faces.size()+num_verts - he.edges.size()/2 << ".\n";
	cerr << unpairedEdges << " unpaired half-edges.\n";
	*/
    cerr << "Done loading face set.\n";
	return true;
}

bool unloadShape(SoIndexedFaceSet *faceset, SoCoordinate3 *coords, Mesh &he) {
	if (!coords || !faceset) return false;
	coords->point.setNum(0);
	int j = 0;
	for (vector<float>::iterator p=he.verts.begin(); p!=he.verts.end(); p+=3) {
		coords->point.set1Value(j++, *p, *(p+1), *(p+2));
	}

	// read half edges into face set
	faceset->coordIndex.setNum(0);
	// loop over faces
	int i = 0;
	for (vector<HE_face *>::iterator p=he.faces.begin(); p!=he.faces.end(); p++) {
		// loop over edges
		HE_edge *edge = (*p)->edge;
		do {
			faceset->coordIndex.set1Value(i++, edge->vert);
			edge = edge->next;
		} while (edge != (*p)->edge);
		faceset->coordIndex.set1Value(i++, -1);
	}
	return true;
}

