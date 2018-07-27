#!/bin/sh -e

# Meant for use on fdroid build servers

# build the tools
CFLAGS=-O2\ -DNDEBUG make -C ../tools simpl.elf makesc.elf
# read kanjivg
../data/kanjivg.py -o kanji.list -p ../data/kanjivg/kanji -t ../tools/simpl.elf
# build assets/kanji.list
../tools/makesc.elf < kanji.list > assets/kanji.list
