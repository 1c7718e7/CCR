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
#include "gfx.h"
#include "std.h"

void lineColor(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 c)
{
#if 0
	rectangleColor(s, x1, y1, x2, y2, c);
#else
	Uint32 color;
	int x, y;
	int dx, dy;
	int err;

	/* assume no clipping is neccesary */

	color = SDL_MapRGBA(s->format, c>>24, c>>16 & 255, c>>8 & 255, c & 255);
	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) > abs(dy)) {
		if (x1 > x2) {
			x = x1; x1 = x2; x2 = x;
			y = y1; y1 = y2; y2 = y;
			dx = -dx;
			dy = -dy;
		}
		const int pixy = s->w*((dy > 0) - (dy < 0));
		dx = abs(dx);
		dy = abs(dy);
		switch (s->format->BytesPerPixel) {
		default:
			DEBUG("%d BytesPerPixel not supported!\n", s->format->BytesPerPixel);
			abort();
		case 4: {
			Uint32 *pix;
			pix = s->pixels;
			pix += x1 + y1*s->w;
			err = dx>>1;
			ran (x, x1, x2) {
				*pix = color;
				pix++;
				err -= dy;
				if (err < 0) {
					pix += pixy;
					err += dx;
				}
			}
			break;
		}
		case 2: {
			Uint16 *pix, p;
			p = *(Uint16*)&color;
			pix = s->pixels;
			pix += x1 + y1*s->w;
			err = dx>>1;
			ran (x, x1, x2) {
				*pix = p;
				pix++;
				err -= dy;
				if (err < 0) {
					pix += pixy;
					err += dx;
				}
			}
		}
		}
	} else {
		if (y1 > y2) {
			x = x1; x1 = x2; x2 = x;
			y = y1; y1 = y2; y2 = y;
			dx = -dx;
			dy = -dy;
		}
		const int pixx = (dx > 0) - (dx < 0);
		dx = abs(dx);
		dy = abs(dy);
		switch (s->format->BytesPerPixel) {
		default:
			DEBUG("%d BytesPerPixel not supported!\n", s->format->BytesPerPixel);
			abort();
		case 4: {
			Uint32 *pix;
			pix = s->pixels;
			pix += x1 + y1*s->w;
			err = dy>>1;
			ran (y, y1, y2) {
				*pix = color;
				pix += s->w;
				err -= dx;
				if (err < 0) {
					pix += pixx;
					err += dy;
				}
			}
			break;
		}
		case 2: {
			Uint16 *pix, p;
			p = *(Uint16*)&color;
			pix = s->pixels;
			pix += x1 + y1*s->w;
			err = dy>>1;
			ran (y, y1, y2) {
				*pix = p;
				pix += s->w;
				err -= dx;
				if (err < 0) {
					pix += pixx;
					err += dy;
				}
			}
		}
		}
	}
#endif
}

void rectangleColor(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 c)
{
	Uint32 color;
	int x, y;

	x1 = max(0, x1);
	x1 = min(s->w, x1);
	x2 = max(0, x2);
	x2 = min(s->w, x2);

	y1 = max(0, y1);
	y1 = min(s->h, y1);
	y2 = max(0, y2);
	y2 = min(s->h, y2);

	if (x1 > x2) {
		x = x1; x1 = x2; x2 = x;
	}
	if (y1 > y2) {
		y = y1; y1 = y2; y2 = y;
	}

	color = SDL_MapRGBA(s->format, c>>24, c>>16 & 255, c>>8 & 255, c & 255);

	switch (s->format->BytesPerPixel) {
	default:
		DEBUG("%d BytesPerPixel not supported!\n", s->format->BytesPerPixel);
		abort();
	case 1:
		{
			Uint8 *pix, p;
			int pixy;

			pixy = s->w - x2 + x1;
			pix = s->pixels;
			p = *(Uint8*)&color;
			ran (y, y1, y2) {
				ran (x, x1, x2) {
					*pix = p;
					pix++;
				}
				pix += pixy;
			}
		}
		break;
	case 4: {
		Uint32 *pix;
		int pixy;

		pixy = s->w - x2 + x1;
		pix = s->pixels;
		pix += + y1*s->w + x1;
		ran (y, y1, y2) {
			ran (x, x1, x2) {
				*pix = color;
				pix++;
			}
			pix += pixy;
		}
		break;
	}
	case 2: {
		Uint16 *pix, p;
		int pixy;

		p = *(Uint8*)&color;
		pixy = s->w - x2 + x1;
		pix = s->pixels;
		pix += + y1*s->w + x1;
		ran (y, y1, y2) {
			ran (x, x1, x2) {
				*pix = p;
				pix++;
			}
			pix += pixy;
		}
		break;
	}
	}
}

