CCR
===

This is a library for recognizing handwritten Chinese characters + an SDL based frontend
and an android app using it.
Recognition is particularly tolerant to incorrect stroke order.

A conversion script for the KanjiVG database of Japanese characters is included.
The recognition code should also work for other similar writing systems.

How to build
============

Build the tools, they don't require any libraries:
```
$ make -C tools simpl.elf makesc.elf
```
Make sure KanjiVG repo is cloned:
```
$ git submodule update --init data/kanjivg
```
Build ./kanji.list from KanjiVG.
```
$ ./data/kanjivg.py
```
./kanji.list should now exist.
Build ./kanji.sc:
```
$ ./tools/makesc.elf < kanji.list > kanji.sc
```

For building the android app:
```
$ cp kanji.sc android/assets/kanji.list
$ cd android
```
Build the library for android:
```
$ ndk-build
```
Proceed with ant as usual, e.g:
```
$ ant debug
```

To build the SDL frontend:
```
$ make -C tools sdl.elf
```
On launch sdl.elf opens `./kanji.sc` (in the current working dir).
Compile `sdl.c` with `-DDB_FILE=<other path>` if desired.
The SDL frontend requires SDL 1.x libraries, edit `data/config.mk` if needed.

Outline of the algorithm
========================

Given a set of handwritten strokes, we compare it against all other sets in the database
which have a similar number of strokes.

Given two sets of strokes (i.e. kanji), `A` and `B`, we construct a complete weighted
bipartite graph (with `A` and `B` as parts) where the weight of an edge `(a, b)` is a
measure of how dissimilar strokes a and b are . Then we find a minimum weight
perfect matching (aka optimal assignment). The weight of this matching is how dissimilar
`A` and `B` is.

Files
=====

- `lib/`
	- `ccr.h`
		- The recognition library.
	- `klist_io.h`
		- Utility for reading `kanji.list`
	- implementation(s) of parts of `ccr.h`, some unused.
- `tools/`
	- References `../lib`
	- `sdl.c`
		- SDL 1.X based frontend, mouse and keyboard, outputs to stdout.
	- `simpl.c`
		- Used by ./data/build.sh to read svg curves from KanjiVG data.
	- `makesc.c`
		- Compiles klist to kanji.sc, which is used by the frontend.
	- Other misc tools.
- `android/`
	- Android frontend.
	- References `../lib`

- `kanji.list`
	- A plaintext file containing simplified kanji strokes.
	- Can be read with `lib/klist_io.h`
	- Each kanji is a unicode codepoint and a list of strokes (in the correct order).
	- Each stroke is a piecewise linear curve described by a sequence of points.
	- Coordinates of all points are in [0,1].
	- Format:
		- <nkanji> <total strokes> <total points>
		- for each kanji
			- <unicode codepoint (in decimal)> <nstrokes>
			- for each stroke (in the proper stroke order)
				- let x = number of points
				- 2x numbers on a single line, separated by a single U+20, U+10 at the end

License
=======

Copyright (C) 2018 Martin Shirokov

CCR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CCR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CCR.  If not, see <http://www.gnu.org/licenses/>.
