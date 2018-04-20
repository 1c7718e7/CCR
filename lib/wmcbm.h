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
#ifndef _WMCBM_H_INCLUDED
#define _WMCBM_H_INCLUDED

#include <stddef.h>

/* Solver for the Weighted Maximum Cardinallity Bipartite Matching problem.
 * also known as the Assignment Problem.
 */

/**
 * @param n number of rows/sources/workers
 * @param m number of columns/sinks/jobs
 * @return bytes needed by wmcbm_solve for a problem of size n*m
 */
size_t wmcbm_mem_needed(int n, int m);

/**
 * @param mem pointer to at least wmcbm_mem_needed(n, m) bytes of memory
 * @param c n-by-m cost matrix, c[i*stride + j] = cost of edge (i, j). All costs must be nonnegative.
 * @param n number of rows/sources/workers
 * @param m number of columns/sinks/jobs
 * @param dst optional array for storing the assignment
 * @return the minimum net cost
 */
double wmcbm_solve(void *mem, const double *c, int n, int m, int stride, int *dst);

#endif /* end of include guard */
