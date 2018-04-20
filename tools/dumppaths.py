#!/usr/bin/env python
# encoding: utf-8
# dump path data from a KanjiVG svg file, to be then read by simpl.c
# dumppaths.py <svg dir> <output dir>

import xml.etree.ElementTree as ET
from sys import argv
from glob import iglob
import re

nl = bytes('\n', 'ascii')

def dump_paths(e, f):
    for c in e:
        if c.tag == '{http://www.w3.org/2000/svg}path':
            f.write(bytes(c.attrib['d'], 'ascii'))
            f.write(nl)
        else:
            dump_paths(c, f)

name = re.compile(".*/([^/]+).svg$")
for f in iglob(argv[1]+"/*.svg"):
    print(f)
    tree = ET.parse(f)
    root = tree.getroot()
    o = open(argv[2] + "/" + name.match(f).group(1), 'wb')
    dump_paths(root, o)
