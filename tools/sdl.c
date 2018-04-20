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
#include "std.h"
#include <SDL.h>
/* #include <SDL_gfxPrimitives.h> */
#include "gfx.h"
#include <signal.h>
#include <errno.h>
#include <math.h>
#include "ccr.h"

/* SDL 1.X frontend
 * Draw with mouse
 * U to erase last stroke
 * C to clear
 * Q to quit
 * any other button to search
 * output is printed to stdout
 */

#ifndef DB_FILE
#	define DB_FILE "./kanji.sc"
#endif

#define WIDTH 400
#define HEIGHT 400
#define NRESULTS 16

static struct {
	uint n, o;
	Vec2 p[KANJI_MAXSAMPLES];
} smp;

static Stroke stroke_buf[KANJI_MAXSTROKES];
static Kanji stk = {0, 0xFFFF, stroke_buf};
static KanjiDB *db;

static SDL_Surface *framebuf;
static char is_dirty;

static int px, py;

static int uenc(u32 c, uchar *u)
{
	if (!(c & (u32)~0x7F)) {
		*u = c;
		return 1;
	} else if (!(c & (u32)~0x7FF)) {
		u[0] = 0xC0 | (c>>6 & 0x1F);
		u[1] = 0x80 | (c    & 0x3F);
		return 2;
	} else if (!(c & (u32)~0xFFFF)) {
		u[0] = 0xE0 | (c>>12 & 0x0F);
		u[1] = 0x80 | (c>>6  & 0x3F);
		u[2] = 0x80 | (c     & 0x3F);
		return 3;
	} else { /* (!(c & (UNICHAR)~0x377777)) { */
		u[0] = 0xF0 | (c>>18 & 0x07);
		u[1] = 0x80 | (c>>12 & 0x3F);
		u[2] = 0x80 | (c>>6  & 0x3F);
		u[3] = 0x80 | (c     & 0x3F);
		return 4;
	}
}

static void do_search()
{
	int i, j;
	KanjiMatch dst[NRESULTS];
	uchar buf[4];

	kanji_normalize(&stk);
	i = kanji_db_lookup(db, &stk, dst, NRESULTS);
	for (j = i-1; j >= 0; j--) {
		printf("%02d ", j);
		fwrite(buf, uenc(dst[j].code, buf), 1, stdout);
		printf(" %lf\n", dst[j].score);
	}
	return;
}

static void reset_samples()
{
	smp.n = smp.o = stk.n = 0;
}

static void pop_stroke()
{
	if (!stk.n)
		return;
	smp.n = smp.o -= stk.p[--stk.n].n;
}

static void push_stroke()
{
	const uint n = smp.n - smp.o;
	if (n < 1)
		return;
	if (stk.n == KANJI_MAXSTROKES) {
		fputs("too many strokes!\n", stderr);
		return;
	}
	stk.p[stk.n] = (Stroke) { n, smp.p + smp.o };
	stroke_simplify(stk.p + stk.n);
	smp.o += stk.p[stk.n].n;
	smp.n = smp.o;
	DEBUG("PUSH STROKE %u  %u, %u\n",
		stk.p[stk.n].n, smp.o, smp.n);
	stk.n++;
}

static void blit_samples()
{
	uint i, j;
	Vec2 a, b;

	if (SDL_MUSTLOCK(framebuf))
		SDL_LockSurface(framebuf);
	SDL_FillRect(framebuf, 0, 0);

	/* the strokes */
	rep (i, stk.n) {
		const Stroke *s = stk.p + i;
		rep (j, s->n-1) {
			a = s->p[j];
			b = s->p[j+1];
			a.x *= WIDTH; a.y *= HEIGHT;
			b.x *= WIDTH; b.y *= HEIGHT;
			lineColor(framebuf, a.x, a.y, b.x, b.y, 0xA0A0A0FF);
			rectangleColor(framebuf, a.x-1, a.y-1, a.x+1, a.y+1, 0xFF0000FF);
		}
		//b = stroke->p[stroke->n-1]; // leftover from above
		rectangleColor(framebuf, b.x-1, b.y-1, b.x+1, b.y+1, 0xFF0000FF);
	}

	/* current line */
	if (smp.n - smp.o < 2)
		return;
	ran (i, smp.o, smp.n-1) {
		a = smp.p[i];
		b = smp.p[i+1];
		a.x *= WIDTH; a.y *= HEIGHT;
		b.x *= WIDTH; b.y *= HEIGHT;
		lineColor(framebuf, a.x, a.y, b.x, b.y, 0xA0A0A0FF);
		//rectangleColor(framebuf, a.x-1, a.y-1, a.x+1, a.y+1, 0xFFFF00FF);
	}
	//b = stroke->p[stroke->n-1]; // leftover from above
	//rectangleColor(framebuf, b.x-1, b.y-1, b.x+1, b.y+1, 0xFFFF00FF);
	if (SDL_MUSTLOCK(framebuf))
		SDL_UnlockSurface(framebuf);
}

#define MINDIST 2
#define MINDISTSQ 4
static void push_point(uint x, uint y)
{
	const int dx = px - x;
	const int dy = py - y;
	const int ss = dx*dx + dy*dy;

	if (smp.n == KANJI_MAXSAMPLES) {
		fputs("too many points!\n", stderr);
		return;
	}
	if (ss < MINDISTSQ)
		return;

	smp.p[smp.n++] = (Vec2) {(double)x/WIDTH, (double)y/HEIGHT};
	px = x; py = y;
	is_dirty = 1;
}
#undef MINDIST
#undef MINDISTSQ

static void mainloop()
{
	SDL_Event e;

	while (SDL_WaitEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			return;
		case SDL_KEYUP:
			if (e.key.keysym.sym == SDLK_q)
				return;
			else if (e.key.keysym.sym == SDLK_u && stk.n > 0)
				pop_stroke();
			else if (e.key.keysym.sym == SDLK_c)
				reset_samples();
			else if (stk.n > 0)
				do_search();
			is_dirty = 1;
			break;
		case SDL_MOUSEBUTTONUP:
			push_stroke();
			is_dirty = 1;
			break;
		case SDL_MOUSEMOTION:
			if (e.motion.state == SDL_PRESSED)
				push_point(e.motion.x, e.motion.y);
			break;
		}
		if (is_dirty) {
			blit_samples();
			SDL_UpdateRect(framebuf, 0, 0, WIDTH, HEIGHT);
			is_dirty = 0;
		}
	}
}

static void open_db()
{
	FILE *f;
	char *data;
	size_t n, m;

	n = 0;
	m = 8*1024*1024;
	data = malloc(m);

	f = cfopen(DB_FILE, "rb");
	for (;;) {
		n += fread(data, 1, m-n, f);
		if (ferror(f))
			exit_perror(DB_FILE);
		if (n == m) {
			data = realloc(data, m *= 2);
		} else
			break;
	}
	fclose(f);
	if (!(db = kanji_db_open(data, n)))
		exit_error("kanji_db_open failed");
	free(data);
}


int main(int argc, char **argv)
{
	open_db();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE))
		goto sdl_err;
	if (signal(SIGINT, SIG_DFL) == SIG_ERR)
		exit_perror("signal");
	reset_samples();
	framebuf = SDL_SetVideoMode(
		WIDTH, HEIGHT, 32,
		SDL_SWSURFACE | SDL_ANYFORMAT |
		0);
	if (!framebuf)
		goto sdl_err;
	mainloop();
	SDL_Quit();
	kanji_db_close(db);
	/* ought to munmap the buf from open_db() */
	return 0;
sdl_err:
	exit_error(SDL_GetError());
}
