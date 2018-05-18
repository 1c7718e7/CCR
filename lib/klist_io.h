/* This file is part of CCR.
 * Copyright (C) 2018  Martin Shirokov
 * 
 * CCR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CCR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CCR.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __IO_H__
#define __IO_H__

/* Functions for reading klist files, e.g. those produced by data/build.sh
   Usage:
   Allocate a struct klist yourself and initialize it with klist_init;
   klist_fread and/or klist_parse can be called any number of times on it;
   results of the previous parse are invalidated by a new parse;
   free heap allocated memory with klist_fini */

#include "ccr.h"

struct klist {
	Vec2 *pointv;
	Stroke *strokev;
	Kanji *kanjiv;
	uint kanjic, strokec, pointc;
	uint kanjim, strokem, pointm;
};
typedef struct klist KList;

int klist_append_copy(KList *kl, const Kanji *k);
int klist_fread(KList *kl, FILE *f);
void klist_fwrite(const KList *kl, FILE *f);
int klist_parse(KList *kl, char **p, char *end);
void klist_init(KList *kl);
void klist_fini(KList *kl);

#endif /* __IO_H__ */
