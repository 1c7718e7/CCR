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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "std.h"

/* random test case generator for assign.c */

#define MAX (100)

#include <fcntl.h>
#include <unistd.h>

static void seed_rand()
{
	int fd, seed;
	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		exit_perror("/dev/urandom");
	read(fd, &seed, sizeof seed);
	close(fd);
	srand(seed);
}

int main(int argc, char **argv)
{
	static double Cost[MAX][MAX];
	seed_rand();
	int n = rand()%MAX+1;
	int m = rand()%MAX+1;
	printf("%i %i\n", n, m);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m-1; j++)
			printf("%lf ", 10*(double)rand()/RAND_MAX);
		printf("%lf\n", 10*(double)rand()/RAND_MAX);
	}
	return 0;
}
