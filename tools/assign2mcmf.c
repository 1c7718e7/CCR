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
#include "wmcbm.h"

int main(int argc, char **argv)
{
	int n, m;
	int i, j;
	double *mat;

	fflush(stderr);
	if (2 != scanf("%i%i", &n, &m))
		goto err_input;
	mat = malloc(n*m*sizeof *mat);
	for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
		if (1 != scanf("%lf", &mat[i*m+j]))
			goto err_input;

	printf("%i %i %i %i\n", n+m+2, n*m + n + m, 0, n+m+1);

	for (i = 0; i < n; i++) {
		printf("0 %i 0\n", i+1);
	}

	for (i = 0; i < n; i++)
	for (j = 0; j < m; j++) {
		printf("%i %i %lf\n", i+1, j+1+n, mat[i*m+j]);
	}

	for (j = 0; j < m; j++) {
		printf("%i %i 0\n", j+1+n, n+m+1);
	}

	return 0;
err_input:
	fprintf(stderr, "error: invalid input\n");
	return 1;
}
