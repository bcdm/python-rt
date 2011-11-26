#!/usr/bin/env python

import rt
import time
import numpy
import gist

gist.pldefault(marks=0)

n=5000
t=0.001

v=numpy.empty(n)

for i in range(n):
  a=time.time()
  time.sleep(t)
  v[i]=time.time()-a

print v.min(),v.max(),v.std()
gist.plg(v,color='red')


tt=long(1e9*t)
v=numpy.empty(n)

rt.sched_setscheduler()
time.sleep(1)
for i in range(n):
  a=time.time()
  rt.clock_nanosleep(tt);
  v[i]=time.time()-a

print v.min(),v.max(),v.std()
gist.plg(v,color='blue')

#v=numpy.empty(n)
def fun(*args, **kwd):
  print 'fun'
#  global v
#  v[i]=time.time()-a
#

import gc
gc.disable()

tsamp=long(1e9)
tsamp=long(500000) # best without 'sched_setscheduler'
tsamp=long(300000) # best with
tsamp=long(200000) # with gc disabled..

tsched = 0
while True:
 tsched,tavail = rt.nanosched(tsched, tsamp, fun)
 print tavail




