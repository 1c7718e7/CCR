CCR
===

This is a library for recognizing handwritten kanji + an SDL based frontend
and an android app using it.

The recognition is particularly tolerant to incorrect stroke order.

Files
=====

lib/
	contains the recognition code and utils used by some tools
tools/
	references ../lib
	sdl.c
		SDL 1.X based frontend; mouse + keyboard; outputs to stdout
	simpl.c
		Used by ./data/build.sh to read svg curves from KanjiVG data
	makesc.c
		Compiles klist to kanji.sc, which is used by the frontend.
	other misc tools
android/
	android frontend
	references ../lib

kanji.list
	A plaintext file containing simplified kanji strokes
	Each kanji is a unicode codepoint and a list of strokes (in the correct order)
	Each stroke is a piecewise linear curve described by a sequence of points.
	Coordinates of all points are in [0,1].
	format
		<nkanji> <total strokes> <total points>
		for each kanji
			<unicode codepoint (in decimal)> <nstrokes>
			for each stroke 
				let x = number of points
				2x numbers on a single line, separated by a single U+20, U+10 at the end

How to build
============

Build the tools, they don't require any libraries:
$ make -C tools simpl.elf makesc.elf
Download the KanjiVG "all" database to ./data:
$ wget https://github.com/KanjiVG/kanjivg/releases/download/r20160426/kanjivg-20160426-all.zip -O data/kanjivg-20160426-all.zip
$ ./data/build.sh
./kanji.list should now exist.
Build kanji.sc:
$ ./tools/makesc.elf < kanji.list > kanji.sc

For building the android app:
$ cp kanji.sc android/assets/kanji.list
$ cd android
Build the library for android:
$ ndk-build
Proceed with ant as usual, e.g:
$ ant debug

Build the SDL frontend, needs SDL 1.X, edit tools/config.mk if needed:
$ make -C tools sdl.elf
On launch sdl.elf opens './kanji.sc' (in current working dir).
Compile sdl.c with -DDB_FILE=<other path> if desired.

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
