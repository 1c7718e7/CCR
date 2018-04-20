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

static KanjiDB *open_db_file(const char *name)
{
	FILE *f;
	char *data;
	size_t n, m;
	KanjiDB *db;

	n = 0;
	m = 8*1024*1024;
	data = malloc(m);

	f = cfopen(name, "rb");
	for (;;) {
		n += fread(data, 1, m-n, f);
		if (ferror(f))
			exit_perror(name);
		if (n == m) {
			data = realloc(data, m *= 2);
		} else
			break;
	}
	fclose(f);
	if (!(db = kanji_db_open(data, n)))
		exit_error("kanji_db_open failed");
	free(data);
	return db;
}

static uint lookupc, successc;

static void test_lookup(KanjiDB *db, Kanji *k)
{
#define MAXRESULTS 16
	static KanjiMatch res[MAXRESULTS];
	uint i, n;

	lookupc++;
	n = kanji_db_lookup(db, k, res, MAXRESULTS);
	rep (i, n) {
		if (res[i].code == k->code) {
			successc++;
			printf("0x%05x %i\n", k->code, i);
			return;
		}
	}
	printf("0x%05x -1\n", k->code);
	fflush(stdout);
}

int main(int argc, char **argv)
{
	KanjiDB *db;
	KList kl;
	FILE *f;
	uint i;

	if (argc != 3)
		exit_error("usage: test_lookup <db> <klist to lookup>");
	db = open_db_file(argv[1]);
	fprintf(stderr, "read db\n");
	f = cfopen(argv[2], "r");
	klist_init(&kl);
	if (klist_fread(&kl, f))
		(ferror(f) ? exit_perror : exit_error)("klist_fread failed");
	fprintf(stderr, "read klist\n");
	fclose(f);

	rep (i, kl.kanjic) {
		test_lookup(db, kl.kanjiv + i);
	}

	printf("%lf success rate\n", (double)successc/lookupc);
	klist_fini(&kl);
	kanji_db_close(db);
	return 0;
}
