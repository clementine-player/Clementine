#!/usr/bin/python

from subprocess import *
from sys import *
from os import rename, remove
from datetime import *

def pretty_years(s):
	
	l = list(s)
	l.sort()

	start = None
	prev = None
	r = []

	for x in l:
		if prev is None:
			start = x
			prev = x
			continue

		if x == prev + 1:
			prev = x
			continue
		
		if prev == start:
			r.append("%i" % prev)
		else:
			r.append("%i-%i" % (start, prev))

		start = x
		prev = x

	if not prev is None:
		if prev == start:
			r.append("%i" % prev)
		else:
			r.append("%i-%i" % (start, prev))

	return ", ".join(r)

def order_by_year(a, b):
	
	la = list(a[2])
	la.sort()
	
	lb = list(b[2])
	lb.sort()

	if la[0] < lb[0]:
		return -1
	elif la[0] > lb[0]:
		return 1
	else:
		return 0

def gen_copyrights(f):

	commits = []
	data = {}
        copyrights = []
        
	for ln in Popen(["git", "blame", "--incremental", f], stdout=PIPE).stdout:
		
		if ln.startswith("filename "):
			if len(data) > 0:
				commits.append(data)
			data = {}

		elif ln.startswith("author "):
			data["author"] = ln[7:].strip()

		elif ln.startswith("author-mail <"):
			data["author-mail"] = ln[12:].strip()

		elif ln.startswith("author-time "):
			data["author-time"] = ln[11:].strip()

		elif ln.startswith("author-tz "):
			data["author-tz"] = ln[9:].strip()

        with open(f,'r') as fi:
                fil = fi.readlines()
                for i in fil:
                        if -1 < i.find("Original Author"):
                                da = i.split("  ")
                                print f
                                copyrights.append("   Copyright %s, %s %s\n" % (da[3].strip(), da[1], da[2]))

        by_author = {}

	for c in commits:
		try:
			n =  by_author[c["author"]]
		except KeyError:
			n = (c["author"], c["author-mail"], set())
			by_author[c["author"]] = n

		# FIXME: Handle time zones properly
		year = datetime.fromtimestamp(int(c["author-time"])).year

		n[2].add(year)

	for an, a in list(by_author.iteritems()):
		for bn, b in list(by_author.iteritems()):
			if a is b:
				continue

			if a[1] == b[1]:
				a[2].update(b[2])

				if by_author.has_key(an) and by_author.has_key(bn):	
					del by_author[bn]

	copyright = list(by_author.itervalues())
	copyright.sort(order_by_year)

	for name, mail, years in copyright:
		copyrights.append("   Copyright %s, %s %s\n" % (pretty_years(years), name, mail))
        return copyrights

def change_file(filename):
        content=[]
        out=[]
        extended=0
        ends=0
        with open(filename, "r") as fi:
                content=fi.readlines()

        copyrights=gen_copyrights(filename)
                
        if -1 == content[0].find("/* This file is part of Clementine."):
                print("File {} have no Clementine copyright info".format(filename))
                return 0

        for i in content:
                if i.find("*/") != -1:
                        ends = 1
                if i.find("Copyright ") != -1:
                        if not extended:
                                out.extend(copyrights)
                                extended = 1
                                
                        if not ends:
                                continue
                else:
                        out.append(i)
        
        with open(filename+'_tmp', "w") as fi:
                fi.writelines(out)
        rename(filename+'_tmp', filename)
        

if __name__ == "__main__":
        for files in argv[1:]:
                change_file(files)
