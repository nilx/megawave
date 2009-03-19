#!/usr/bin/env python
#
"""Usage: %s [options] <nccout> <function> ...
valid options:
\t -h | --help: usage statement
\t -V | --version: print version and exit
\t -D | --dot: additional options to dot
\t -d | --depth: max depth of graph
\t -i | --ignore: functions to ignore
\t -l | --location: show file names of function declarations
\t -s | --show: show function but ignore sub-functions

Reads a ncc-generated file and produces a call graph in dot format.
Sample usage:
gengraph.py -i "exit strlen" nccout main |dot -Tsvg -o func.svg
"""
#
# Author: Jose Vasconcellos <jvasco@bellatlantic.net>
# Copyright (C) 2004 Jose Vasconcellos
#
# gengraph.py is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# gengraph.py is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Tested with Python version 2.3 and ncc-1.9

import sys
import os
import string
import getopt

# globals
location = 0
maxdepth = 1000        # large number
dot = None
ignore = []
show = []
shown = []

cfile = None    #current file
cfunc = None    #current func
rfunc = {}

# funcs: function dictionary
# this dictionary maps a function to a list containing dependencies
funcs = {}

####################

def usage():
    print __doc__ % os.path.basename(sys.argv[0])
    sys.exit(1)

####################

def addfunc(f, d):
    if f in funcs:
        # already exists, just append
        funcs[f].append(d)
    else:
        # new function entry; 1st item holds file name
        funcs[f] = [None, d]

def addfile(f, p):
    if f in funcs:
        # existing entry, put file name in 1st entry in list
        funcs[f][0] = p
    else:
        # new function entry; create list with file name
        funcs[f] = [p]

####################

def word(w):
    return w.rstrip(" ()\t\n")

####################

def gengraph(s, d=0):
    if s:
        shown.append(s)
    if s in funcs:
        l = funcs[s]
        for f in l[1:]:
            f1 = f
            edge = ""
            if f1[0] == '*':
                edge = "[color=blue]"
                while rfunc.has_key(f1):
                    f1 = rfunc[f1]
            print '"%s" -> "%s" %s;' % (s, f1, edge)
            if d < maxdepth and f1 == f and f not in shown:
                gengraph(f,d+1)

####################
# start of main program

# process command line options
try:
    opts, args = getopt.getopt(sys.argv[1:], "hvVD:d:i:l:s:", \
	["help","version","depth","dot","ignore","location","show"])

except getopt.GetoptError:
    # print help information and exit:
    usage()

for o, a in opts:
    if o in ("-h", "--help"):
	usage()
    elif o in ("-V", "--version"):
        print "$Id: gengraph.py 5 2004-11-18 03:21:58Z Jose $"
	sys.exit(1)
    elif o in ("-i", "--ignore"):
	ignore = string.split(a)
    elif o in ("-D", "--dot"):
	dot = a
    elif o in ("-d", "--depth"):
	maxdepth = int(a)
    elif o in ("-l", "--location"):
	location = 1
    elif o in ("-s", "--show"):
	show = string.split(a)

if len(args) < 2:
    usage()

infile = open(args[0])
start = args[1:]

# process lines in nccout file
# and populate dictionary
for line in infile:
    if len(line) == 0:
        pass
    elif line[0] == 'D':
        w = word(line[3:])
        if w not in ignore:
            cfunc = w
        cfile = None
    elif line[0] == 'F':
        if cfunc and cfunc not in show:
            w = word(line[3:])
            if w not in ignore:
                addfunc(cfunc, w)
        cfile = None
    elif line[0] == 'P':
        cfile = line[3:]
        cfunc = None
    elif line[0] == 'Y':
        addfile(word(line[3:]), cfile)
        cfunc = None
    elif line[0] == 'R':
        l = line[3:].split()
        rfunc[word(l[0])] = word(l[1])
        cfunc = None
        cfile = None

# generate graph
print "digraph gengraph {"
if dot:
    print dot

# traverse graph
for s in start:
    gengraph(s)

if location:
    pass        #todo
else:
    # mark special nodes
    for s in shown:
        if s in show:
            print '"%s" [ style="bold", color="red", shape="box" ];' % s
        if s[0] == '*':
            print '"%s" [ shape="hexagon" ];' % s
        elif s.find('/') >= 0:
            print '"%s" [ shape="octagon" ];' % s
    # mark start nodes
    for s in start:
        print '"%s" [ style="filled,bold", fillcolor="#dddddd", shape="box" ];' % s

print "}"

# vim: set ai sw=4 et :
