#ifndef  __VEC2F_H__
#define  __VEC2F_H__

#include <cmath>
#include <iostream>

template <class t> struct Vec2 {
	union {
		struct {t x,y;};
		t raw[2];
	};

	Vec2() : x(0), y(0) {}
	Vec2(const Vec2<t> &v) : x(v.x), y(v.y) {}
	Vec2<t>& operator =(const Vec2<t>& v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
		}
		return *this;
	}
	Vec2(const t *p) : x(p[0]), y(p[1]) {}
	Vec2(t _x, t _y) : x(_x),y(_y) {}
	Vec2<t> operator -() {return Vec2(-x, -y);}
	const Vec2<t>& operator *=(t f) { x*=f; y*=f; return *this; }
	const Vec2<t>& operator /=(t f) { x/=f; y/=f; return *this; }
	const Vec2<t>& operator +=(const Vec2<t> &v) { x+=v.x; y+=v.y; return *this; }
	const Vec2<t>& operator -=(const Vec2<t> &v) { x-=v.x; y-=v.y; return *this; }
	Vec2<t> operator *(t f) const { return Vec2<t>(x*f, y*f); }
	Vec2<t> operator /(t f) const { return Vec2<t>(x/f, y/f); }
	Vec2<t> operator +(const Vec2<t> &v) const { return Vec2<t>(x+v.x, y+v.y); }
	Vec2<t> operator -(const Vec2<t> &v) const { return Vec2<t>(x-v.x, y-v.y); }
	t operator *(const Vec2<t> &v) const { return x*v.x + y*v.y; }
	t norm () const { return sqrt(x*x+y*y); }
	void normalize(t l=1) { *this *= l/norm(); }

	double angle(Vec2<t> v) {
		double res = acos((*this/norm())*(v/v.norm()));
		if (x*v.y - y*v.x<0) return -res;
		return res;
	}

	void rotate(double angle) {
		t ox = x, oy=y;
		x = ox*cos(angle) - oy*sin(angle);
		y = ox*sin(angle) + oy*cos(angle);
	}

	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")";
	return s;
}

#endif // __VEC2F_H__

