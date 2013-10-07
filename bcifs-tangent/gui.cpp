#undef NDEBUG
#include <assert.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "vec2.h"

#include <GL/glut.h>

using namespace std;
float zoom = 1.f;
Vec2<float> displacement(0, 0);
Vec2<float> lastpos(0,0);

GLfloat viewport[4];

int niter = 8;
int selected =  0; // none:0
int selectid = -1; // 0: lm   1:alpha

float mu = .5;
float lm = .5;
float alpha = -1;

Vec2<float> polygon[] = {Vec2<float>(-.5, -.5), Vec2<float>(0, .5), Vec2<float>(.5, -.5)};
vector<Vec2<float> > curve;

Vec2<float> lm_origin = Vec2<float>(-.5, .5);
float lm_unit = .43;
int points_not_lines = 0;

Vec2<float> alpha_origin = Vec2<float>(0, -.83);
float alpha_unit = .43;


Vec2<float> screen2space(Vec2<float> p) {
	return Vec2<float>(zoom*((p.x-displacement.x)*2.0/viewport[2]-1.0), zoom*(1.0-(p.y-displacement.y)*2.0/viewport[3]));
}

Vec2<float> space2screen(Vec2<float> p) {
	return Vec2<float>((p.x/zoom+1.0)*viewport[2]/2.0+displacement.x, (1.0-p.y/zoom)*viewport[3]/2.0+displacement.y);
}

Vec2<float> T0(float a, float b, float c, Vec2<float> *pt) {
	float p[] = {a+b*(1-lm)+c*(1-lm)*alpha/(alpha-1), b*lm+c*(lm*alpha-mu)/(alpha-1), c*(mu-1)/(alpha-1)};
	return pt[0]*p[0] + pt[1]*p[1] + pt[2]*p[2];
}

Vec2<float> T0(float *w, Vec2<float> *pt) {
	return T0(w[0], w[1], w[2], pt);
}

Vec2<float> T1(float a, float b, float c, Vec2<float> *pt) {
	float p[] = {a*(1-lm)*alpha/(alpha-1), a*(lm*alpha-mu)/(alpha-1)+mu*b, a*(mu-1)/(alpha-1)+(1-mu)*b+c};
	return pt[0]*p[0] + pt[1]*p[1] + pt[2]*p[2];
}

Vec2<float> T1(float *w, Vec2<float> *pt) {
	return T1(w[0], w[1], w[2], pt);
}


void render2dtext(Vec2<float> p, string text, bool align_center=true) {
	int blen = glutBitmapLength(GLUT_BITMAP_9_BY_15, (const unsigned char *)text.c_str());
	glRasterPos2f(p.x-(align_center?blen/2.0*zoom*2.0/viewport[2]:0), p.y);
	for (int i = 0; i < (int)text.length(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text.c_str()[i]);
	}
}

void subdivide() {
	curve.clear();
	for (int i=0; i<3; i++) curve.push_back(polygon[i]);
	for (int i=0; i<niter; i++) {
		vector<Vec2<float> > tmp;
		for (int j=0; j<(int)curve.size(); j+=3) {
			tmp.push_back(T0(1, 0, 0, curve.data()+j));
			tmp.push_back(T0(0, 1, 0, curve.data()+j));
			tmp.push_back(T0(0, 0, 1, curve.data()+j));
			tmp.push_back(T1(1, 0, 0, curve.data()+j));
			tmp.push_back(T1(0, 1, 0, curve.data()+j));
			tmp.push_back(T1(0, 0, 1, curve.data()+j));
		}
		curve = tmp;
	}
}

void render_scene(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(1);
	glBegin(GL_POINTS);
	const float step = .01;
	for (float l=-1; l<1; l+=step) {
		for (float m=-1; m<1; m+=step) {
			if (fabs(l)>fabs((m-1)/(alpha-1))) {
				glColor3f(.1, .1, .3);
				glVertex2fv(space2screen(Vec2<float>(l,m)*lm_unit+lm_origin+Vec2<float>(step/2, step/2)).raw);
			}
			if (fabs(m)>fabs((1-l)*alpha/(alpha-1))) {
				glColor3f(.1, .3, .1);
				glVertex2fv(space2screen(Vec2<float>(l,m)*lm_unit+lm_origin).raw);
			}
			if (fabs((1-l)*alpha/(alpha-1))>1 || fabs((m-1)/(alpha-1))>1) {
				glColor3f(.2, .03, .03);
				glVertex2fv(space2screen(Vec2<float>(l,m)*lm_unit+lm_origin+Vec2<float>(step/3, step/3)).raw);
			}
		}
	}
	glEnd();

	glColor3f(.1, .1, .1);
	glBegin(GL_LINES);
		glVertex2fv(space2screen(lm_origin - Vec2<float>(lm_unit, 0)).raw);
		glVertex2fv(space2screen(lm_origin + Vec2<float>(lm_unit, 0)).raw);
		glVertex2fv(space2screen(lm_origin - Vec2<float>(0, lm_unit)).raw);
		glVertex2fv(space2screen(lm_origin + Vec2<float>(0, lm_unit)).raw);
		glVertex2fv(space2screen(alpha_origin - Vec2<float>(alpha_unit, 0)).raw);
		glVertex2fv(space2screen(alpha_origin + Vec2<float>(alpha_unit, 0)).raw);
	glEnd();
	glPointSize(5);
	{
		if (selected && !selectid) {
			glColor3f(.1, .9, .1);
		} else {
			glColor3f(.9, .9, .9);
		}
		glBegin(GL_POINTS);
			glVertex2fv(space2screen(Vec2<float>(lm, mu)*lm_unit + lm_origin).raw);
		glEnd();

		stringstream ss;
		ss.setf(std::ios_base::fixed);
		ss.precision(2);
		ss << "(" << lm << ", " << mu << ")";
		render2dtext(space2screen(Vec2<float>(lm, mu)*lm_unit+lm_origin+Vec2<float>(.02, -.01)), ss.str());
	}

	{
		if (selected && selectid) {
			glColor3f(.1, .9, .1);
		} else {
			glColor3f(.9, .9, .9);
		}
		glBegin(GL_POINTS);
			glVertex2fv(space2screen(Vec2<float>(alpha*alpha_unit, 0) + alpha_origin).raw);
		glEnd();

		stringstream ss;
		ss.setf(std::ios_base::fixed);
		ss.precision(2);
		ss << alpha;
		render2dtext(space2screen(Vec2<float>(alpha*alpha_unit, 0)+alpha_origin+Vec2<float>(.02, -.01)), ss.str());
	}


	glColor3f(.1, .1, .1);

	glBegin(GL_LINE_STRIP);
	for (int i=0; i<3; i++) {
		Vec2<float> z = space2screen(polygon[i]);
		glVertex2fv(z.raw);
	}
	glEnd();

	float lt[][3] = {{-1, 1, 0}, {1, -(alpha*lm - mu)/(lm - 1), ((alpha - 1)*lm - mu + 1)/(lm - 1)}};
	int li = fabs(lm)<fabs((mu-1)/(alpha-1));
	float rt[][3] = {{0, 1, -1}, {1, -(alpha*lm - mu)/(alpha*lm + (alpha - 1)*mu - alpha), -(alpha*mu - alpha)/(alpha*lm + (alpha - 1)*mu - alpha)}};

	int ri = fabs(mu)<fabs((1-lm)*alpha/(alpha-1));
	Vec2<float> lp = T0(lt[li], polygon);
	lp.normalize(.3);
	Vec2<float> rp = T1(rt[ri], polygon);
	rp.normalize(.3);

	Vec2<float> cr = T0(rt[ri], polygon);
	cr.normalize(.3);

	Vec2<float> cl = T1(lt[li], polygon);
	cl.normalize(.3);


	glLineWidth(2.0);
	glBegin(GL_LINES);
		glColor3f(.5, .7, .5);
		glVertex2fv(space2screen(polygon[0]).raw);
		glVertex2fv(space2screen(polygon[0]+lp).raw);
		glVertex2fv(space2screen(T1(1, 0, 0, polygon)).raw);
		glVertex2fv(space2screen(cl+T1(1, 0, 0, polygon)).raw);

		glColor3f(.5, .5, .7);
		glVertex2fv(space2screen(polygon[2]).raw);
		glVertex2fv(space2screen(polygon[2]+rp).raw);
		glVertex2fv(space2screen(T0(0, 0, 1, polygon)).raw);
		glVertex2fv(space2screen(cr+T0(0, 0, 1, polygon)).raw);
	glEnd();

	glColor3f(.9, .1, .1);
	glLineWidth(1.0);
	if (points_not_lines) {
		glPointSize(1);
		glBegin(GL_POINTS);
	} else {
		glBegin(GL_LINE_STRIP);
	}
	for (int i=0; i<(int)curve.size(); i++) {
		Vec2<float> z = space2screen(curve[i]);
		glVertex2fv(z.raw);
	}
	glEnd();



	glFlush();
	glutSwapBuffers();
}


void resize(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGetFloatv(GL_VIEWPORT, viewport);
}

void change_zoom(bool in) {
	zoom/=(in ? 1.2 : .8);
	GLfloat viewport[4];
	glGetFloatv(GL_VIEWPORT, viewport);
	resize(viewport[2], viewport[3]);
	glutPostRedisplay();
}

void process_keyboard(unsigned char key, int x, int y) {
	if (27==key) {
		exit(0);
	}
	if ('*'==key) {
		zoom = 1.;
		displacement = Vec2<float>(0, 0);
	}
	if ('+'==key) {
		niter++;
		subdivide();
	}
	if ('-'==key) {
		niter--;
		subdivide();
	}
	if ('p'==key) {
		points_not_lines = 1-points_not_lines;
	}
	glutPostRedisplay();
}

void process_mouse_motion(int x, int y) {
	if (!selected) {
		displacement += Vec2<float>(x, y)-lastpos;
		lastpos = Vec2<float>(x, y);
	} else {
		Vec2<float> p = screen2space(Vec2<float>(x, y));
		if (0==selectid) {
			Vec2<float> t = (p-lm_origin)/lm_unit;
			lm = t.x;
			mu = t.y;
		} else if (1==selectid){
			alpha = (p.x-alpha_origin.x)/alpha_unit;
		}

		subdivide();
	}
	glutPostRedisplay();
}

void process_passive_mouse_motion(int x, int y) {
	selectid = -1;
	selected = 0;
	const float selection_distance = 5;
	Vec2<float> p = Vec2<float>(x, y);

	if (Vec2<float>(p-space2screen(Vec2<float>(lm, mu)*lm_unit + lm_origin)).norm()<selection_distance) {
		selected = 1;
		selectid = 0;
	} else if (Vec2<float>(p-space2screen(Vec2<float>(alpha*alpha_unit, 0) + alpha_origin)).norm()<selection_distance) {
		selected = 1;
		selectid = 1;
	}
	glutPostRedisplay();
}

void process_mouse(int button, int state, int x, int y) {
	lastpos = Vec2<float>(x, y);
	if (3==button) {
		change_zoom(true);
	}
	if (4==button) {
		change_zoom(false);
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 800);
	glutCreateWindow("bcifs");

	glutDisplayFunc(render_scene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(process_keyboard);
	glutMouseFunc(process_mouse);
	glutPassiveMotionFunc(process_passive_mouse_motion);
	glutMotionFunc(process_mouse_motion);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	subdivide();
	glutMainLoop();
	return 0;
}

