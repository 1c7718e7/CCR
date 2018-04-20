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
#include <errno.h>
#include "ccr.h"

int main(int argc, char **argv)
{
	FILE *f;
	char *data;
	size_t n, m;
	uint iter;

	n = 0;
	m = 8*1024*1024;
	data = malloc(m);

	f = cfopen(argc == 2 ? argv[1] : "kanji.list", "rb");
	for (;;) {
		n += fread(data, 1, m-n, f);
		if (ferror(f))
			exit_perror(argc == 2 ? argv[1] : "kanji.list");
		if (n == m) {
			data = realloc(data, m *= 2);
		} else
			break;
	}
	fclose(f);

	printf("read %zu bytes\n", n);
	fflush(stdout);
	rep (iter, 1024) {
		KanjiDB *db;
		if (!(db = kanji_db_open(data, n)))
			exit_error("kanji_db_open failed");
		kanji_db_close(db);
	}
	printf("parsed %u times\n", iter);
	fflush(stdout);
	free(data);
	return 0;
}
