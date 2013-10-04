#!/usr/bin/python
import random
from numpy import *
import matplotlib.pyplot as plt



[l0,l1,l2] = [7, .7, -.4]
[r0,r1,r2] = [.3, .3, .3]

lmbd = .75
mu = .5



seed = random.randint(0,110000)
random.seed(seed)
print seed

[a0, a1, a2] = [1., 0., 0.]
[c0, c1] = [random.random(), random.random()]
c2 = 1.-c0-c1

[e0, e1] = [(mu*r0-r0*c1)/r1, (mu*r1-r0*c1)/r1]
e2 = 1.-e0-e1

[b0,b1] = [((1.-lmbd)*(1.-l0)-c0*l2)/l1, (lmbd*l1-c1*l2)/l1]
b2 = 1.-b0-b1

[d0, d1, d2] = [c0, c1, c2]

[f0, f1, f2] = [0., 0., 1.]


#T0=matrix([[a0,b0,c0],[a1,b1,c1],[a2,b2,c2]], dtype=float);
#[c0, c1, c2] = [random.random(), random.random(), 0]#[(1-lmbd)/2, (mu+lmbd)/2, (1-mu)/2]
[c0, c1, c2] = [(1-lmbd)/2, (mu+lmbd)/2, (1-mu)/2]
c2 = 1 - c0 - c1
print c0+c1+c2
T0=matrix([[1.,1-lmbd,c0],[0, lmbd, c1],[0,0,c2]], dtype=float);
#T1=matrix([[d0,e0,f0],[d1,e1,f1],[d2,e2,f2]], dtype=float);
T1=matrix([[c0,0,0],[c1,mu,0],[c2,1-mu,1]], dtype=float);
P=matrix([[0., .5, 1.],[0.,1.,0.]])
x=[]
y=[]

maxdepth = 14

def apply(C, depth):
	if depth<maxdepth:
		apply(C*T0, depth+1)
		apply(C*T1, depth+1)
	else:
		foo = P*C*matrix([[1],[0],[0]],dtype=float)
		x.append(foo[0,0])
		y.append(foo[1,0])


apply(matrix(identity(3),dtype=float), 0)

print T0
print
print T1

#plt.plot([P[0,0], l0*P[0,0]+l1*P[0,1]+l2*P[0,2]], [P[1,0], l0*P[1,0]+l1*P[1,1]+l2*P[1,2]], 'g')
#plt.plot([P[0,2], r0*P[0,0]+r1*P[0,1]+r2*P[0,2]], [P[1,2], r0*P[1,0]+r1*P[1,1]+r2*P[1,2]], 'g')
plt.plot([P[0,0], P[0,1], P[0,2]], [P[1,0], P[1,1], P[1,2]], 'b')
plt.plot(x, y, 'r')
plt.show()



