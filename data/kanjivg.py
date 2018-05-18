#!/bin/env python3
# encoding: utf-8

# Script for converting a KanjiVG database to klist.
# We collect svg path data from all *.svg files and pipe it to ./tools/simpl.elf

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-p", "--path", dest="path", default="data/kanjivg/kanji",
                  help="read svg files from PATH", metavar="PATH")
parser.add_option("-o", "--out", dest="out", default="kanji.list",
                  help="write klist to FILE", metavar="FILE")
parser.add_option("-t", "--tool", dest="tool", default="./tools/simpl.elf",
                  help="tool for reading svg paths", metavar="EXECUTABLE")
(opt, args) = parser.parse_args()

import xml.etree.ElementTree as ET
from glob import iglob
import re
import os
import subprocess as subp
import sys
import io

def paths_from_file(f):
	path_tag = '{http://www.w3.org/2000/svg}path'
	tree = ET.parse(f)
	root = tree.getroot()
	ps = map(lambda p: p.attrib['d'], root.iter(path_tag))
	return list(ps)

filename_re = re.compile("([0-9a-fA-F]+)(-[^/]+)?.svg$")

kanji_cnt = 0
for _ in filter(filename_re.match, os.listdir(opt.path)):
	kanji_cnt += 1

out = open(opt.out, "wb")
conv = subp.Popen([opt.tool], stdin=subp.PIPE, stdout=out)
inp = io.TextIOWrapper(conv.stdin, 'ascii')

inp.write("{}\n".format(kanji_cnt))

for fpath in sorted(os.listdir(opt.path)):
	match = filename_re.match(fpath)
	if not match:
		continue
	code = int(match.group(1), 16)
	with open(os.path.join(opt.path, fpath), "r") as f:
		ps = paths_from_file(f);
		inp.write("{} {}\n".format(code, len(ps)))
		for p in ps:
			inp.write(p)
			inp.write('\n')

inp.close()
if conv.wait() != 0:
	raise RuntimeError("{} exited with non-zero status".format(opt.tool))
out.close()
