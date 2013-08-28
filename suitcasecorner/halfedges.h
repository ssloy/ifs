#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <iostream>
#include <vector>

using namespace std;

struct HE_face;

typedef struct HE_edge {
	HE_edge();
	HE_edge(int _vert, HE_face *_face);
	int vert;
//	int orig;   // debugging
	HE_edge *pair;
	HE_face *face;
	HE_edge *next;
	HE_edge *prev;
	bool mark;
} HE_edge;

typedef struct HE_face {
	HE_face();
	HE_edge *edge;
	int size;
} HE_face;

class Mesh {
private:
	vector<HE_edge *> edges;
	vector<HE_face *> faces;
	vector<float> verts;
public:
	void clear_edge_marks();
	Mesh();
	~Mesh();
	bool doo_sabin(vector<vector<float> > coeffs);
	friend std::ostream& operator<<(std::ostream& s, Mesh& he);
	friend bool unloadShape(SoIndexedFaceSet *shape, SoCoordinate3 *coords, Mesh &he);
	friend bool   loadShape(SoIndexedFaceSet *shape, SoCoordinate3 *coords, Mesh &he);
};



