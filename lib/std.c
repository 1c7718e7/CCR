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
#include <string.h>
#include <stdarg.h>

void exit_perror(str m, ...)
{
	va_list ap;
	va_start(ap, m);
	fflush(stdout);
	vfprintf(stderr, m, ap);
	fputs(": ", stderr);
	va_end(ap);
	perror("");
	exit(1);
}

void exit_error(str s, ...)
{
	va_list ap;
	va_start(ap, s);
	fflush(stdout);
	vfprintf(stderr, s, ap);
	fputc('\n', stderr);
	va_end(ap);
	exit(1);
}

void exit_rerror(str s)
{
	fprintf(stderr, "%s: ", s);
	if (errno)
		exit_perror("short read");
	else
		exit_error("short read");
}

FILE *cfopen(str path, str mode)
{
	FILE *f = fopen(path, mode);
	if (!f)
		exit_perror(path);
	return f;
}

void cfread(void *buf, size_t n, FILE *f)
{
	if (1 != fread(buf, n, 1, f))
		exit_rerror("error");

}

void *xalloc(void *a, size_t s)
{
	void *v = realloc(a, s);
	if (s && !v)
		abort();
	return v;
}

long fsize(FILE *f)
{
	long s;
	if (-1 == fseek(f, 0, SEEK_END)
	 || -1 == (s = ftell(f))
	 || -1 == fseek(f, 0, SEEK_SET))
                return -1;
        else
                return s;
}

long freadall(void **dst, FILE *f)
{
	long s;
        if (-1 == (s = fsize(f)))
                return -1;
	*dst = malloc(s);
        if (!*dst)
                return -1;
        if (1 == fread(*dst, s, 1, f))
                return s;
        free(*dst);
        return -1;
}
