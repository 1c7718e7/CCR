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
#ifndef _CCR_H_INLUDED
#define _CCR_H_INLUDED

#include "std.h"

/* base structs & utils for cleaning input data */

#define KANJI_MAXSAMPLES 256
#define KANJI_MAXSTROKES 32

typedef struct { double x, y; } Vec2;

typedef struct stroke {
	uint n;
	Vec2 *p;
} Stroke;

typedef struct {
	uint n;
	u32 code;
	Stroke *p;
} Kanji;

void stroke_simplify(Stroke *s);
void kanji_normalize(Kanji *k);

/* Kanji-Kanji metric */

typedef struct kanji_dist KanjiDist;

KanjiDist *kanji_dist_new();
void kanji_dist_free(KanjiDist *d);
double kanji_dist(KanjiDist *d, const Kanji *a, const Kanji *b);

/* Stroke-Stroke metric */

typedef struct stroke_dist StrokeDist;

StrokeDist *stroke_dist_new();
void stroke_dist_free(StrokeDist *d);
double stroke_dist(StrokeDist *d, const Stroke *a, const Stroke *b);

/* Lookup */

typedef struct kanji_db KanjiDB;

typedef struct kanji_match {
	double score;
	u32 code;
	void *cookie; /* for passing to kanji_db_data */
} KanjiMatch;

KanjiDB *kanji_db_open(char *data, size_t len);
void kanji_db_close(KanjiDB *db); /* the above FILE is NOT closed */
/* return at most n best matches is res, sorted starting with the best one */
uint kanji_db_lookup(KanjiDB *db, const Kanji *k, KanjiMatch *res, uint n);
const Kanji *kanji_db_data(KanjiDB *db, void *cookie);
void kanji_db_data_release(KanjiDB *db, void *cookie, const Kanji *k);

/* Feedback */

void kanji_feedback(const Kanji *user, const Kanji *model, int *ui, int *mi);
	/* ui[i] = best match for user[i] */
	/*       = -1, if no match */
	/* ditto mi[i] */

#endif
