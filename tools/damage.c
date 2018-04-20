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
#include "ccr.h"
#include "klist_io.h"
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

/* add noise to all points in input klist */

static void seed_rand()
{
	int seed;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &seed, sizeof seed);
	close(fd);
	srand(seed);
}

static double frand()
{
	return (double)rand()/RAND_MAX;
}

static double noisef()
{
	double a = 0;
	uint i;
	rep (i, 8)
		a += frand();
	return ((a/8*2) - .5)*.2;
}

static double clip(double x)
{
	return min(1, max(0, x));
}

int main(int argc, char **argv)
{
	KList kl;
	uint i, j, t;

	seed_rand();
	klist_init(&kl);
	if (klist_fread(&kl, stdin))
		(ferror(stdin) ? exit_perror : exit_error)("klist_fread failed");
	printf("%u %u %u\n", kl.kanjic, kl.strokec, kl.pointc);
	rep (i, kl.kanjic) {
		Kanji *const k = kl.kanjiv + i;
		printf("%u %u\n", k->code,  k->n);
		rep (j, k->n) {
			rep (t, k->p[j].n-1)
				printf("%lf %lf ", clip(k->p[j].p[t].x + noisef()), clip(k->p[j].p[t].y + noisef()));
			printf("%lf %lf\n", clip(k->p[j].p[t].x + noisef()), clip(k->p[j].p[t].y + noisef()));
		}
	}
	klist_fini(&kl);
	return 0;
}
