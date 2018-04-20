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

/* foolproof implementation of min cost max flow; for testing purposes */
/* input format:
 * <number of nodes> <number of edges> <source> <sink>
 * <source> <dest> <cost>
 * nodes are zero indexed, cost is a decimal fraction
 * no parallel edges are allowed
 */

#define MAX (1000)

static int nodecnt;
static double Cost[MAX][MAX];
static int Cap[MAX][MAX];
static int source, sink;

static struct dpcell {
	double cost;
	int prev;
} Dp[MAX][MAX];

static void floyd_warshall()
{
	for (int i = 0; i < nodecnt; i++)
	for (int j = 0; j < nodecnt; j++) {
		Dp[i][j].cost = Cap[i][j] ? Cost[i][j] : HUGE_VAL;
		Dp[i][j].prev = i;
	}

	for (int k = 0; k < nodecnt; k++)
	for (int i = 0; i < nodecnt; i++)
	for (int j = 0; j < nodecnt; j++) {
		if (Dp[i][k].cost + Dp[k][j].cost < Dp[i][j].cost) {
			Dp[i][j].cost = Dp[i][k].cost + Dp[k][j].cost;
			Dp[i][j].prev = Dp[k][j].prev;
		}
	}
}

static void augment(int from, int to)
{
	while (from != to) {
		int k = Dp[from][to].prev;
		assert(k != to);
		assert(Cap[k][to]);
		assert(!Cap[to][k]);
		Cap[k][to] = 0;
		Cap[to][k] = 1;
		to = k;
	}
}

int main(int argc, char **argv)
{
	int edgecnt;

	scanf("%i%i%i%i", &nodecnt, &edgecnt, &source, &sink);
	for (int i = 0; i < edgecnt; i++) {
		int from, to;
		double cost;
		scanf("%i%i%lf", &from, &to, &cost);
		if (from == to)
			exit_error("loops not allowed");
		if (Cap[from][to])
			exit_error("parallel edges not allowed");
		if (Cap[to][from])
			exit_error("antiparallel edges not allowed");
		Cap[from][to] = 1;
		Cost[from][to] = cost;
		Cost[to][from] = -cost;
	}

	double cost = 0;
	int flow = 0;
	for (;;) {
		floyd_warshall();
		if (Dp[source][sink].cost == HUGE_VAL)
			break;
		augment(source, sink);
		flow++;
		cost += Dp[source][sink].cost;
		printf("flow %i cost %lf\n", flow, cost);
	}
	printf("final flow %i cost %lf\n", flow, cost);
	return 0;
}
