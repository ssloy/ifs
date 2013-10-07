#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/draggers/SoDragPointDragger.h>
#include <Inventor/draggers/SoTranslate2Dragger.h>


#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>

#include "halfedges.h"
SoDragPointDragger *pd = NULL;
SoTranslate2Dragger *td = NULL;
SbVec3f scaleFactor;
Mesh he;
SoGroup *mutants  = NULL;
SoIndexedFaceSet *shape = NULL;
SoCoordinate3 *coords   = NULL;
int niterations = 2;

// тут внимание, коэффициенты упакованы для обхода граней по кругу, в то время как в
// моих матрицах для обычной грани a b d c
vector<vector<float> > coeffs;

void init() {
	coeffs.clear();
	niterations = 2;
	vector<float> four;
//	four.push_back(.5625);
//	four.push_back(.1875);
//	four.push_back(.0625);
//	four.push_back(.1875);
	four.push_back(.111);
	four.push_back(.222);
	four.push_back(.445);
	four.push_back(.222);
	vector<float> three;
//	three.push_back(.66666);
//	three.push_back(.166666);
//	three.push_back(.166666);
	three.push_back(.75);
	three.push_back(.125);
	three.push_back(.125);
	coeffs.push_back(three);
	coeffs.push_back(four);
	if (pd) {
//		pd->translation.setValue(coeffs[1][1]/scaleFactor[0], coeffs[1][0]/scaleFactor[1], coeffs[1][3]/scaleFactor[2]);
		pd->translation.setValue(2.65165, 5.625, 0);
	}
	if (td) {
		td->translation.setValue(0, 2.357, 0);
	}
}

static char * buffer;
static size_t buffer_size = 0;
static void * buffer_realloc(void * bufptr, size_t size) {
	buffer = (char *)realloc(bufptr, size);
	buffer_size = size;
	return buffer;
}
static SbString buffer_writeaction(SoNode * root) {
	SoOutput out;
	buffer = (char *)malloc(1024);
	buffer_size = 1024;
	out.setBuffer(buffer, buffer_size, buffer_realloc);

	SoWriteAction wa(&out);
	wa.apply(root);

	SbString s(buffer);
	free(buffer);
	return s;
}


bool subdivide(vector<vector<float> > &coeffs) {
	if (!mutants || !shape || !coords) return false;
	mutants->removeAllChildren();
	SoSeparator *res = new SoSeparator();
	SoCoordinate3 * c = new SoCoordinate3();
	SoIndexedFaceSet *s = new SoIndexedFaceSet();
	loadShape(shape, coords, he);
	for (int i=0; i<niterations; i++) {
		he.doo_sabin(coeffs);
	}
	unloadShape(s, c, he);
	res->addChild(c);
	res->addChild(s);
	mutants->addChild(res);
	return true;

}

static void event_cb(void * ud, SoEventCallback * n) {
	SoQtExaminerViewer * viewer = (SoQtExaminerViewer *)ud;
	if (SO_KEY_PRESS_EVENT(n->getEvent(), R)) {
		init();
		subdivide(coeffs);
		return;
	}
	if (SO_KEY_PRESS_EVENT(n->getEvent(), PAD_ADD)) {
		niterations++;
		subdivide(coeffs);
		return;
	}
	if (SO_KEY_PRESS_EVENT(n->getEvent(), PAD_SUBTRACT)) {
		niterations--;
		subdivide(coeffs);
		return;
	}
	if (SO_KEY_PRESS_EVENT(n->getEvent(), O)) {
		std::cout << he;

	}
	if (SO_KEY_PRESS_EVENT(n->getEvent(), I)) {
//		SbString s = buffer_writeaction(mutants);//viewer->getSceneGraph());
		SbString s = buffer_writeaction(viewer->getSceneGraph());
		stringstream ss;
		ss << "out_a=" << coeffs[1][0] << ",b=" << coeffs[1][1] << ",c=" << coeffs[1][3] << ", f=" << coeffs[0][1] << ",g=" << coeffs[0][2] << ".wrl";
		ofstream outfile;
		outfile.open (ss.str().c_str());
		outfile << "#VRML V1.0 ascii" << endl;
		outfile << s.getString();
		outfile.close();
//		(void)fprintf(stdout, "%s\n", s.getString());
		return;
	}
}

bool extract_pointers(SoSeparator *scene, SoIndexedFaceSet *&shape, SoCoordinate3 *&coords) {
	SoSearchAction sa;
	sa.setType(SoIndexedFaceSet::getClassTypeId());
	sa.setInterest(SoSearchAction::ALL);
	sa.apply(scene);
	SoPathList plist = sa.getPaths();
	int len = plist.getLength();
	cerr << "Searching for IndexedFaceSet node...";
	if (len<=0) {
		cerr << "Could not find SoIndexedFaceSet\n";
		return false;
	}
	cerr << "found.\n";

	SoPath *path = (SoPath*) plist[0];
	shape = (SoIndexedFaceSet *)path->getTail();
	SoGroup *shapeParent = (SoGroup *)path->getNodeFromTail(1);
	cerr << "Searching for Coordinate3 node...";
	sa.setType(SoCoordinate3::getClassTypeId());
	sa.setSearchingAll(FALSE); // don't look under off switches
	sa.setInterest(SoSearchAction::LAST);
	sa.apply(shapeParent);

	path = sa.getPath();
	if (!path) {
		cerr << "Could not find Coordinate3 node!\n";
		return false;
	}
	cerr << "found.\n";
	coords = (SoCoordinate3 *)path->getTail();
	return true;
}

// Callback for the point draggers.
void draggerCB(void *data, SoSensor *sensor) {
	if (!sensor) return;
	SoField *field = ((SoFieldSensor *)sensor)->getAttachedField();
	if (!field) return;
	SbVec3f  trans = ((SoSFVec3f *)field)->getValue();
	trans[0] *= scaleFactor[0]; // Scale down translations
	trans[1] *= scaleFactor[1];
	trans[2] *= scaleFactor[2];
	if (!data) {
		// 2д драггер повёрнут в .iv файле дважды; поэтому его translation координаты не совпадают с мировыми, необходимо повернуть на 45 градусов, чтобы совпали
		float f = trans[0]/sqrt(2) + trans[1]/sqrt(2);
		float g = trans[1]/sqrt(2) - trans[0]/sqrt(2);
		float e = 1-f-g;
		cerr << "f: " << f << ", g: " << g << endl;
		coeffs[0][0] = e;
		coeffs[0][1] = f;
		coeffs[0][2] = g;
	} else {
		float a = trans[1];
		float b = trans[0]/sqrt(2) - trans[2]/sqrt(2);
		float c = trans[0]/sqrt(2) + trans[2]/sqrt(2);
		coeffs[1][0] = a;
		coeffs[1][1] = b;
		coeffs[1][2] = 1-a-b-c;
		coeffs[1][3] = c;

		cerr << "a: " << a << ", b:" << b << ", c:" << c << ", d:" << (1-a-b-c) << endl;
//		cerr << fabs(2*d + 2*a -1) << endl;
//		cerr << sqrt((1-a-2*b-d)*(1-a-2*b-d) + (a-d)*(a-d)) << endl;
	}

	subdivide(coeffs);
}

int main(int argc, char *argv[]) {
	init();
	QWidget *myWindow = SoQt::init(argv[0]);
	if (myWindow == NULL) exit(1);

	SoQtExaminerViewer *myViewer = new SoQtExaminerViewer(myWindow);
	SoInput in;
	SoSeparator *root = NULL;

	if (in.openFile("draggers.iv")) {
		root = SoDB::readAll(&in);
		if (!root) {
			return 1;
		}
	}
	root->ref();

	SoSearchAction sa;
	sa.setType(SoScale::getClassTypeId());
	sa.setInterest(SoSearchAction::LAST);
	sa.apply(root);

	SoPath *path = sa.getPath();
	cerr << "Searching for SoScale node...";
	if (!path) {
		cerr << "Could not find SoScale node!\n";
		return 1;
	}
	cerr << "found.\n";
	SoScale *scale = (SoScale *)path->getTail();
	scaleFactor = scale->scaleFactor.getValue();

	sa.setType(SoDragPointDragger::getClassTypeId());
	sa.apply(root);
	path = sa.getPath();
	cerr << "Searching for SoDragPointDragger node...";
	if (!path) {
		cerr << "Could not find SoDragPointDragger node!\n";
		return 1;
	}
	cerr << "found.\n";
	pd = (SoDragPointDragger *)path->getTail();
	SoFieldSensor *fieldsensor = new SoFieldSensor(draggerCB, pd);
	fieldsensor->attach(&pd->translation);

	sa.setType(SoTranslate2Dragger::getClassTypeId());
	sa.apply(root);
	path = sa.getPath();
	cerr << "Searching for SoTranslate2Dragger node...";
	if (!path) {
		cerr << "Could not find SoTranslate2Dragger node!\n";
		return 1;
	}
	cerr << "found.\n";
	td = (SoTranslate2Dragger *)path->getTail();
	SoFieldSensor *fieldsensor2 = new SoFieldSensor(draggerCB, NULL);
	fieldsensor2->attach(&td->translation);

	SoEventCallback * ecb_k = new SoEventCallback;
	ecb_k->addEventCallback(SoKeyboardEvent::getClassTypeId(), event_cb, myViewer);
	root->addChild(ecb_k);

	SoEventCallback * ecb_m = new SoEventCallback;
	ecb_m->addEventCallback(SoMouseButtonEvent::getClassTypeId(), event_cb, myViewer);
	root->addChild(ecb_m);

	mutants = new SoGroup;
	root->addChild(mutants);

	SoSeparator *model = NULL;
	if (in.openFile(1<argc?argv[argc-1]:"model.iv")) {
		model = SoDB::readAll(&in);
		if (!model) {
			return 1;
		}
	}
	model->ref();

	if (!extract_pointers(model, shape, coords)) {
		cerr << "failed to extract pointers" << endl;
		return 1;
	}
	subdivide(coeffs);

	myViewer->setSceneGraph(root);
	myViewer->setTitle("Slider Box");
	myViewer->show();
	SoQt::show(myWindow);
	myViewer->viewAll();
	SoQt::mainLoop();
	model->unref();
	return(0);
}
