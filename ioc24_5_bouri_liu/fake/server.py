#!/usr/bin/env python
import os, time

s2fName = '/tmp/s2f_bl'
if not os.path.exists(s2fName):
   os.mkfifo(s2fName)
s2f = open(s2fName,'w+')

f2sName = '/tmp/f2s_bl'
if not os.path.exists(f2sName):
   os.mkfifo(f2sName)
f2s = open(f2sName,'r')

# s2f.write("w hello\n")
# s2f.flush()
str = f2s.readline()
print '%s' % str,
s2f.write(str)
s2f.flush()

f2s.close()
s2f.close()
