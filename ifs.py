#!/usr/bin/python
import random
from numpy import *
import matplotlib.pyplot as plt



[l0,l1,l2] = [.4, .8, -.2]
[r0,r1,r2] = [.5, .5, 0]

#random.seed(1)

[c0, c1] = [random.random(), random.random()]
[e0, e1] = [random.random(), random.random()]
[f0, f1] = [random.random(), random.random()]
c2 = 1.-c0-c1
e2 = 1.-e0-e1
f2 = 1.-f0-f1



a0=                -c0*(l2/l0 - l1*(l2*r0/l0 - r2)/(l0*(l1*r0/l0 - r1))) + e0*(l1**2/(l0*(l1*r0/l0 - r1)) - l1*r1/((l1*r0/l0 - r1)*r0)) + f0*(l1*l2/(l0*(l1*r0/l0 - r1)) - l1*r2/((l1*r0/l0 - r1)*r0)) + l1/(l1*r0/l0 - r1) - l1*r0/(l0*(l1*r0/l0 - r1)) + 1;
a1 = -c1*(l2/l0 - l1*(l2*r0/l0 - r2)/(l0*(l1*r0/l0 - r1))) + e1*(l1**2/(l0*(l1*r0/l0 - r1)) - l1*r1/((l1*r0/l0 - r1)*r0)) + f1*(l1*l2/(l0*(l1*r0/l0 - r1)) - l1*r2/((l1*r0/l0 - r1)*r0)) + l1/l0 - l1**2*r0/(l0**2*(l1*r0/l0 - r1)) + l1*r1/((l1*r0/l0 - r1)*r0);
a2 = 1.-a0-a1
b0=       -e0*(l1/(l1*r0/l0 - r1) - l0*r1/((l1*r0/l0 - r1)*r0)) - f0*(l2/(l1*r0/l0 - r1) - l0*r2/((l1*r0/l0 - r1)*r0)) - (l2*r0/l0 - r2)*c0/(l1*r0/l0 - r1) - l0/(l1*r0/l0 - r1) + r0/(l1*r0/l0 - r1);
b1=                                             -e1*(l1/(l1*r0/l0 - r1) - l0*r1/((l1*r0/l0 - r1)*r0)) - f1*(l2/(l1*r0/l0 - r1) - l0*r2/((l1*r0/l0 - r1)*r0)) - (l2*r0/l0 - r2)*c1/(l1*r0/l0 - r1) + l1*r0/(l0*(l1*r0/l0 - r1)) - l0*r1/((l1*r0/l0 - r1)*r0);
b2 = 1.-b0-b1
d0 =                                                                                                                                                                                                                               -r1*e0/r0 - r2*f0/r0 + 1;
d1=                                                                                                                                                                                                                             -r1*e1/r0 - r2*f1/r0 + r1/r0;
d2 = 1.-d0-d1


T0=matrix([[a0,b0,c0],[a1,b1,c1],[a2,b2,c2]], dtype=float);
T1=matrix([[d0,e0,f0],[d1,e1,f1],[d2,e2,f2]], dtype=float);
P=matrix([[0., .5, 1.],[0.,1.,0.]])
x=[]
y=[]

maxdepth = 14

def apply(C, depth):
	if depth<maxdepth:
		apply(C*T0, depth+1)
		apply(C*T1, depth+1)
	else:
		foo = P*C*matrix([[l0],[l1],[l2]],dtype=float)
		x.append(foo[0,0])
		y.append(foo[1,0])


apply(matrix(identity(3),dtype=float), 0)

print T0
print
print T1

plt.plot([P[0,0], P[0,1], P[0,2]], [P[1,0], P[1,1], P[1,2]], 'b')
plt.plot(x, y, 'r')
plt.show()



