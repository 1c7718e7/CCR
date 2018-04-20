#!/bin/sh -e

# assumes kanjivg-*-all.zip in already downloaded
# and that tools/* are built
# to be run from repo root

cd data

rm -rf kanji paths simpl
mkdir paths simpl
unzip $(ls kanjivg-*-all.zip | head -n1)

# extract the paths
../tools/dumppaths.py kanji paths
# simplify and convert them
ls paths/* | awk -F'[/-]' '{c="../tools/simpl.elf 0x"$2" < "$0" > simpl/"$2"-"$3;print c;system(c)}'
# remove duplicates and concat
cat $(sha1sum simpl/* | sort | uniq -w 40 | cut -d\  -f3)  > kanji.tmp
# create the header
awk '{nk++;ns+=$2;n=$2;while(n--){getline;np += NF/2}}END{print nk, ns, np}' < kanji.tmp > kanji.list
cat kanji.tmp >> kanji.list
mv kanji.list ..
