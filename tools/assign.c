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

/* tests wmcbm code */

int main(int argc, char **argv)
{
	int n, m;
	int i, j;
	int err;
	double *mat, cost, mycost;
	void *mem;
	int *dst;
	int *cnt;
	int asn;

	fprintf(stderr, "reading...\n");
	fflush(stderr);
	if (2 != scanf("%i%i", &n, &m))
		goto err_input;
	fprintf(stderr, "alloc %zu bytes...\n", n*m*sizeof *mat);
	mat = malloc(n*m*sizeof *mat);
	for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
		if (1 != scanf("%lf", &mat[i*m+j]))
			goto err_input;

	fprintf(stderr, "solving...\n");
	fflush(stderr);
	mem = malloc(wmcbm_mem_needed(n, m));
	dst = malloc(n*sizeof *dst);
	fprintf(stderr, "alloc %zu bytes...\n", wmcbm_mem_needed(n, m) + n*sizeof *dst);
	cost = wmcbm_solve(mem, mat, n, m, m, dst);
	free(mem);

	fprintf(stderr, "sanity check...\n");
	fflush(stderr);
	err = 0;
	mycost = 0;
	asn = 0;
	for (i = 0; i < n; i++) if (dst[i] != -1) {
		mycost += mat[i*m + dst[i]];
		asn++;
	}
	if (n < m && asn != n || m < n && asn != m) {
		fprintf(stderr, "error: impossible max flow: %i\n", asn);
		err++;
	}
	if (fabs(mycost-cost) > 0.0000001) {
		fprintf(stderr, "error: returned cost (%lf) disagrees with assignment (%lf)\n", cost, mycost);
		err++;
	}
	cnt = malloc(m*sizeof *cnt);
	for (j = 0; j < m; j++)
		cnt[j] = 0;
	for (i = 0; i < n; i++) if (dst[i] != -1)
		cnt[dst[i]]++;
	for (j = 0; j < m; j++) if (cnt[j] > 1) {
		fprintf(stderr, "error: right node %i used %i times\n", j, cnt[j]);
		err++;
	}
	free(cnt);
	free(mat);
	if (err) return 1;

	fprintf(stderr, "writing...\n");
	fflush(stderr);
	printf("%.6lf\n", cost);
	for (i = 0; i < n; i++) {
		printf("%i\n", dst[i]);
	}
	free(dst);
	return 0;
err_input:
	fprintf(stderr, "error: invalid input\n");
	return 1;
}
