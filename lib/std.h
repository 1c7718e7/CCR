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
#ifndef STD_H
#define STD_H

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define i8  int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define i8l  int_least8_t
#define i16l int_least16_t
#define i32l int_least32_t
#define i64l int_least64_t

#define u8l  uint_least8_t
#define u16l uint_least16_t
#define u32l uint_least32_t
#define u64l uint_least64_t

#define i8f  int_fast8_t
#define i16f int_fast16_t
#define i32f int_fast32_t
#define i64f int_fast64_t

#define u8f  uint_fast8_t
#define u16f uint_fast16_t
#define u32f uint_fast32_t
#define u64f uint_fast64_t

typedef const char *str;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
#define ran(_v, _a, _b) for (_v = _a; _v < _b; _v++)
#define rep(_v,_n) ran(_v,0,_n)

#ifdef __cplusplus
# define CPP_WRAP extern "C" {
# define CPP_UNWRAP }
#else
# define CPP_WRAP
# define CPP_UNWRAP
#endif

union word {
	intptr_t i;
	uintptr_t u;
	void *p;
};

void *xalloc(void *a, size_t s);
void exit_perror(str m, ...);
void exit_error(str s, ...);
void exit_rerror(str s);
FILE *cfopen(str path, str mode);
void cfread(void *buf, size_t n, FILE *f);
long fsize(FILE *f);
long freadall(void **dst, FILE *f);

#define arraysizeof(x) (sizeof(x)/sizeof *(x))
#define min(x,y) ((x) > (y) ? (y) : (x))
#define max(x,y) ((x) < (y) ? (y) : (x))

#ifndef NDEBUG
# define DEBUG(...) fprintf(stderr, "[DEBUG] "__VA_ARGS__)
#else
# define DEBUG(...) {}
#endif

#define RESIZE_BUF(_b, _n, _m, _i)\
	if ((_n) + (_i) > (_m)) {\
		while ((_n) + (_i) == (_m))\
			(_m) *= 2;\
		(_b) = xalloc((_b), (_m) * sizeof *(_b));\
	}\

#endif /* end of include guard: STD_H */

