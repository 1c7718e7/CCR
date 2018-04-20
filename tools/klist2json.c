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

int main(int argc, char **argv)
{
	KList kl;
	uint i, j, k;

	klist_init(&kl);
	if (klist_fread(&kl, stdin))
		(ferror(stdin) ? exit_perror : exit_error)("klist_fread failed");
	puts("[");
	rep (i, kl.kanjic) {
		const Kanji *const kj = kl.kanjiv+i;
		puts("{");
		printf("\"code\": %"PRIu32",\n", kj->code);
		printf("\"strokes\": [\n", kj->code);
		rep (j, kj->n) {
			const Stroke *const s = kj->p+j;
			puts("[");
			rep (k, s->n) {
				printf("[%lf,%lf]", s->p[k].x, s->p[k].y);
				if (k+1 < s->n)
					puts(",");
			}
			puts("]");
			if (j+1 < kj->n)
				puts(",");
		}
		puts("]}");
		if (i+1 < kl.kanjic)
			puts(",");
	}
	puts("]");
	klist_fini(&kl);
	return 0;
}
