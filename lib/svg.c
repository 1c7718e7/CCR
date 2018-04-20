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
#include "svg.h"

void svg_start(SvgState *s, SvgCallback cb, void *user)
{
	s->cmd = 0;
	s->prev = 0;
	s->args[0] = 0;
	s->args[1] = 0;
	s->cb = cb;
	s->user = user;
}

#define ST_NONE 0
#define ST_INT  1
#define ST_FRAC 2

int svg_feedc(SvgState *s, char in)
{
	unsigned tmp;
	double f;

	if (!s->cmd)
		goto in_cmd;
in_num:
	switch (in) {
	case '\n':
	case ' ':
	case ',':
		if (s->num.st == ST_NONE)
			goto ret_ok;
		else
			goto do_num;
	case '.':
		if (s->num.st == ST_FRAC)
			goto do_num;
		s->num.st = ST_FRAC;
		goto ret_ok;
	case '-':
		if (s->num.st != ST_NONE)
			goto do_num;
		s->num.s = 1;
		s->num.st = ST_INT;
		goto ret_ok;
	/* case 'e': is this in the spec? */
	default:
		tmp = in-'0';
		if (tmp > 9) {
			if (s->num.st == ST_NONE)
				goto in_cmd;
			else
				goto do_num;
		} else {
			if (s->num.st == ST_FRAC) {
				s->num.f = s->num.f*10 + tmp;
				s->num.e *= 10;
			} else {
				s->num.i = s->num.i*10 + tmp;
				s->num.st = ST_INT;
			}
			goto ret_ok;
		}
	}
do_num:
	f = s->num.i + (double)s->num.f/s->num.e;
	if (s->num.s)
		f = -f;
	if (s->cmd&32)
		f += s->args[s->nargs&1];
	s->args[s->nargs] = f;
	if (++s->nargs == s->margs) {
		/* TODO S and Q */
		switch (s->cmd|32) {
		case 'm':
			goto cmd_nop;
		case 's':
			if (s->prev != 's' && s->prev != 'c') { 
				s->args[2] = s->args[4];
				s->args[3] = s->args[5];
			}
			break;
		}
		s->cb(s->user, s->args, s->nargs>>1);
cmd_nop:
		s->args[0] = s->args[s->margs-2];
		s->args[1] = s->args[s->margs-1];
		s->args[2] = s->args[4];
		s->args[3] = s->args[5];
		s->prev = s->cmd|32;
		s->nargs = (s->cmd|32) == 's' ? 4 : 2; /* kinda ugly */
	}
	s->num.s = s->num.i = s->num.f = 0;
	s->num.st = ST_NONE;
	s->num.e = 1;
	goto in_num;
in_cmd:
	s->cmd = in;
	s->nargs = 2;
	switch (s->cmd|32) {
	case 'm':
		s->margs = 4;
		break;
	case 'l':
		s->margs = 4;
		break;
	case 's':
		s->nargs = 4;
	case 'c':
		s->margs = 8;
		break;
	default:
		goto ret_err;
	}
	/* reset s->num */
	s->num.s = s->num.i = s->num.f = 0;
	s->num.st = ST_NONE;
	s->num.e = 1;
	goto ret_ok;
ret_ok:
	return 0;
ret_err:
	return 1;
}

int svg_feeds(SvgState *s, char *buf, unsigned n)
{
	unsigned i;

	for (i = 0; i < n; i++)
		if (svg_feedc(s, buf[i]))
			return i+1;
	return 0;
}

#ifdef COMPILE_EXAMPLE

#include <stdio.h>
#include <string.h>

static void test_callback(void *_, double *p, unsigned n)
{
	unsigned i;

	printf("%u ", n);
	for (i = 0; i < n; i++)
		printf(" %lf,%lf", p[i*2], p[i*2+1]);
	putchar('\n');
}

int main(int argc, char *argv[])
{
	SvgState svg;
	int r, l, c;

	memset(&svg, 0, sizeof svg);
	svg_start(&svg, test_callback, NULL);
	l = c = 1;
	while ((r = getchar()) != EOF) {
		if (r == '\n')
			l++, c = 0;
		else
			c++;
		//r = svg_feeds(&svg, argv[1], strlen(argv[1]));
		r = svg_feedc(&svg, r);
		if (r)
			goto err;
	}
	return 0;
err:
	fprintf(stderr, "error in line %d column %d\n", l, c);
	return 1;
}

#endif
