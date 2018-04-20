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
#include <math.h>

/* read a matrix of pairwise distances and test some properties */

static double mat[12000][12000];
static uint N;

#define EPS 0.001

static void read_mat(FILE *f)
{
	uint i, j;

	fprintf(stderr, "reading... ");
	fflush(stderr);
	if (1 != fscanf(f, "%u",  &N))
		exit_error("invalid format");
	if (N > 12000)
		exit_error("too big");
	rep (i, N)
	rep (j, N)
		fscanf(f, "%lf", &mat[i][j]);
	fprintf(stderr, "done\n");
}

static void check_subadd()
{
	uint i, j, k;
	ulong err;
	fprintf(stderr, "checking subadditivity... ");
	fflush(stderr);
	err = 0;
	rep (i, N)
	rep (j, N)
	rep (k, N) if (mat[i][k]-EPS > mat[i][j] + mat[j][k])
		err++;
	fprintf(stderr, "done\n");
	printf("subadditivity violations: %lu/%lu = %lf%%\n", err, (ulong)N*N*N, 100*(double)err/((double)N*N*N));
}

static void check_symmetry()
{
	uint i, j;
	ulong err;
	fprintf(stderr, "checking symmetry... ");
	fflush(stderr);
	err = 0;
	rep (i, N)
	rep (j, N) if (fabs(mat[i][j] - mat[i][j]) > .0001)
		err++;
	fprintf(stderr, "done\n");
	printf("symmetry violations: %lu/%lu = %lf%%\n", err, (ulong)N*N, 100*(double)err/((double)N*N));
}

static void check_reflexivity()
{
	uint i, j;
	ulong err;
	fprintf(stderr, "checking reflexivity... ");
	fflush(stderr);
	err = 0;
	rep (i, N)
	rep (j, N) {
		if (i != j && mat[i][j] < .001)
			err++;
		if (i == j && mat[i][j] > .001)
			err++;
	}
	fprintf(stderr, "done\n");
	printf("reflexivity violations: %lu/%lu = %lf%%\n", err, (ulong)N*N, 100*(double)err/((double)N*N));
}


int main(int argc, char **argv)
{
	read_mat(stdin);
	check_subadd();
	check_symmetry();
	check_reflexivity();
	return 0;
}
