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

#include "wmcbm.h"
#include <math.h>
#include <stddef.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

#ifndef NDEBUG
# include <stdio.h>
# define DEBUG(...) fprintf(stderr, "[wmcbm] "__VA_ARGS__)
#elif 1
# define DEBUG(...) ((void)0)
#endif

/* Successive shortest paths type algorithm, using Dijkstra and node potentials
   O(n*n*n) time; O(n) memory
   about the algorithm: https://www.topcoder.com/community/data-science/data-science-tutorials/minimum-cost-flow-part-two-algorithms/
*/

size_t wmcbm_mem_needed(int n, int m)
{
	return (n+m)*sizeof(double) /* potential array */
	     + (n+m)*sizeof(double) /* new potential array */
	     + (n+m)*sizeof(int) /* other node array */
	     + (n+m)*sizeof(int) /* parent node array */
	     + (n+m)*sizeof(int) /* Dijkstra queue */
	;
}

double wmcbm_solve(void *_mem, const double *c, int n, int m, int stride, int *dst)
{
#define C(i, j) c[(i)*stride+(j)]
	int i, j, k, tmp, flow, maxflow, cur, prev, mi;
	double pc;
	double cost;
	double d;
	double *D, *N;
	double *TMP;
	int *P, *Q, *L;

	D = _mem; /* distance from source aka node potentials */
	N = D+n+m; /* new potentials */
	P = (int*)(N+n+m); /* prev node on the shortest path from source */
	Q = P+n+m; /* O(n) priority queue */
	L = Q+n+m; /* node (from the other partition) matched to */

	/* init dist[] and parent[] */
	for (j = 0; j < m; j++)
		D[n+j] = HUGE_VAL, P[n+j] = n+j;
	for (k = 0; k < n+m; k++)
		Q[k] = k, L[k] = -1;
	/* there are no backwards edges yet */
	for (i = 0; i < n; i++) {
		D[i] = 0.0, P[i] = i;
		for (j = 0; j < m; j++) if (D[n+j] > C(i, j)) {
			D[n+j] = C(i, j);
			P[n+j] = i;
		}
	}

	maxflow = n < m ? n : m;
	cost = 0.0;
	for (flow = 0; flow < maxflow; flow++) {
		DEBUG("flow = %i; cost = %lf\n", flow, cost);
		/* find nearest free rightside node */
		mi = -1;
		for (j = 0; j < m; j++) if (L[j+n] < 0) {
			mi = j+n;
			break;
		}
		assert(mi != -1); /* otherwise it would be flow >= maxflow */
		for (j = mi-n+1; j < m; j++) if (L[j+n] < 0 && D[mi] > D[j+n])
			mi = j+n;
		cur = mi;
		prev = P[cur];
		if (prev == cur) {
			DEBUG("maxflow = %i\n", flow);
			cost = HUGE_VAL;
			assert(D[mi] == HUGE_VAL);
			goto end;
		}
		do {
			if (prev < cur) {
				DEBUG("path: %i --> %i  %lf\n", prev, cur, C(prev, cur-n));
				L[cur] = prev;
				L[prev] = cur;
				cost += C(prev, cur-n);
			} else {
				DEBUG("path: %i <-- %i  %lf\n", cur, prev, -C(cur, prev-n));
				cost -= C(cur, prev-n);
			}
			cur = prev;
			prev = P[cur];
		} while (prev != cur);

		/* recalculate D[] into N[] */
		for (j = 0; j < m; j++)
			N[j+n] = HUGE_VAL, P[j+n] = j+n;
		for (i = 0; i < n; i++) {
			N[i] = L[i] < 0 ? 0.0 : HUGE_VAL;
			P[i] = i;
		}
		for (k = 0; k < n; k++) {
			DEBUG("================ k = %i\n", k);
			for (i = 0; i < n; i++)
				DEBUG("L %2i: N=%.2lf\t P=%i D=%lf\n", i, N[i], P[i], D[i]);
			for (i = n; i < n+m; i++)
				DEBUG("R %2i: N=%.2lf\t P=%i D=%lf\n", i, N[i], P[i], D[i]);
			mi = k;
			for (i = k+1; i < n; i++) if (N[Q[mi]] > N[Q[i]])
				mi = i;
			DEBUG("mi = %i; SELECT %i; N = %lf\n", mi, Q[mi], N[Q[mi]]);
			if (N[Q[mi]] == HUGE_VAL)
				break;
			tmp = Q[k]; Q[k] = Q[mi]; Q[mi] = tmp;
			#ifndef NDEBUG
			DEBUG("Q:");
			for (i = k; i < n+m; i++)
				fprintf(stderr, " %i", Q[i]);
			fprintf(stderr, "\n");
			#endif
			/* now Q[:k] is visited nodes and Q[k:] is unvisited,
			   Q[k] being nearest */
			for (j = 0; j < m; j++) if (j+n != L[Q[k]]) {
				d = C(Q[k], j) + D[Q[k]] - D[j+n];
				//assert(d >= 0.0);
				if (N[j+n] > N[Q[k]] + d) {
					N[j+n] = N[Q[k]] + d;
					P[j+n] = Q[k];
					DEBUG("N[%i] <- %lf\n", j+n, N[j+n]);
					DEBUG("P[%i] <- %i\n", j+n, Q[k]);
				}
				if (L[j+n] != -1) {
					assert(L[j+n] < n);
					d = -(C(L[j+n], j) + D[L[j+n]] - D[j+n]);
					if (N[L[j+n]] > N[j+n] + d) {
						N[L[j+n]] = N[j+n] + d;
						P[L[j+n]] = j+n;
						DEBUG("N[%i] <- %lf\n", L[j+n], N[L[j+n]]);
						DEBUG("P[%i] <- %i\n", L[j+n], j+n);
					}
				}
			}
		}
		for (i = 0; i < n+m; i++)
			D[i] += N[i];
		//TMP = D; D = N; N = TMP;
	}
end:
	if (dst) {
		for (i = 0; i < n; i++)
			dst[i] = L[i] == -1 ? -1 : L[i]-n;
	}
	DEBUG("flow = %i; cost = %lf\n", flow, cost);
	return cost;
}
