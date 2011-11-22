#!/usr/bin/env python

import rt
import time
import numpy
import gist

gist.pldefault(marks=0)

n=60000
t=0.001

v=numpy.empty(n)

for i in range(n):
  a=time.time()
  time.sleep(t)
  b=time.time()
  v[i]=b-a

print v.min(),v.max(),v.std()
gist.plg(v,color='red')


tt=long(1e9*t)
v=numpy.empty(n)

rt.sched_setscheduler()
time.sleep(1)
for i in range(n):
  a=time.time()
  rt.clock_nanosleep(tt);
  b=time.time()
  v[i]=b-a

print v.min(),v.max(),v.std()
gist.plg(v,color='blue')




