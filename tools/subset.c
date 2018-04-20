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

/* write a klist, that is a random subset of input klist */

static void seed_rand()
{
	int seed;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &seed, sizeof seed);
	close(fd);
	srand(seed);
}

int main(int argc, char **argv)
{
	KList kl;
	uint i, j, t;
	uint kc, sc, pc;
	uint *perm;

	if (argc != 2)
		exit_error("usage: subset <cardinality>");
	kc = atoi(argv[1]);
	seed_rand();
	klist_init(&kl);
	if (klist_fread(&kl, stdin))
		(ferror(stdin) ? exit_perror : exit_error)("klist_fread failed");
	if (kc > kl.kanjic) {
		fprintf(stderr, "warning: more kanji requested than in klist\n");
		kc = kl.kanjic;
	}
	perm = malloc(kl.kanjic * sizeof *perm);
	rep (i, kl.kanjic)
		perm[i] = i;
	rep (i, kc) {
		j = rand()%(kl.kanjic-i) + i;
		t = perm[i];
		perm[i] = perm[j];
		perm[j] = t;
	}
	sc = pc = 0;
	rep (i, kc) {
		Kanji *const k = kl.kanjiv + perm[i];
		sc += k->n;
		rep (j, k->n)
			pc += k->p[j].n;
	}
	printf("%u %u %u\n", kc, sc, pc);
	rep (i, kc) {
		Kanji *const k = kl.kanjiv + perm[i];
		printf("%u %u\n", k->code,  k->n);
		rep (j, k->n) {
			rep (t, k->p[j].n-1)
				printf("%lf %lf ", k->p[j].p[t].x,  k->p[j].p[t].y);
			printf("%lf %lf\n", k->p[j].p[t].x,  k->p[j].p[t].y);
		}
	}
	klist_fini(&kl);
	return 0;
}
