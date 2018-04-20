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
#ifndef SVG_H
#define SVG_H

/* A parser for a subset of SVG path data */

/* p contains the n coefficients of parsed bezier curve */
typedef void (*SvgCallback)(void *u, double *p, unsigned n);
typedef struct {
	char cmd, prev;
	double args[8];
	unsigned nargs, margs;
	SvgCallback cb;
	void *user;
	struct {
		unsigned s, i, e, f;
		char st;
	} num;
} SvgState;

void svg_start(SvgState *s, SvgCallback cb, void *user);
int svg_feeds(SvgState *s, char *buf, unsigned n);
int svg_feedc(SvgState *s, char in);

#endif /* end of include guard: SVG_H */

