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
package shirokov.martin.ccr;

import java.util.Arrays;

public class Kanji {
	public double[] pointv;
	public int[] sizev;
	public int pointc, sizec;

	public Kanji(double[] pv, int[] sv)
	{
		assert(pv != null);
		assert(sv != null);
		pointv = pv;
		pointc = pointv.length;
		sizev = sv;
		sizec = sizev.length;
	}

	public Kanji()
	{
		pointv = new double[1024];
		sizev = new int[32];
	}

	private Kanji(Kanji k)
	{
		int i, n;

		n = pointc = k.pointc;
		pointv = new double[n];
		for (i = 0; i < n; i++)
			pointv[i] = k.pointv[i];

		n = sizec = k.sizec;
		sizev = new int[n];
		for (i = 0; i < n; i++)
			sizev[i] = k.sizev[i];
	}

	public Kanji clone()
	{
		return new Kanji(this);
	}

	public void pushStroke()
	{
		if (sizec >= sizev.length) {
			int[] t = new int[sizev.length*2];
			for (int i = 0; i < sizec; i++)
				t[i] = sizev[i];
			sizev = t;
		}
		sizev[sizec++] = 0;
	}

	public void pushPoint(double x, double y)
	{
		assert(sizec > 0);
		if (pointc >= pointv.length) {
			double[] t = new double[pointv.length*2];
			for (int i = 0; i < pointc; i++)
				t[i] = pointv[i];
			pointv = t;
		}
		pointv[pointc++] = x;
		pointv[pointc++] = y;
		sizev[sizec-1]++;
	}

	public void endStroke()
	{
		if (sizec > 1 && sizev[sizec-1] <= 2) {
			popStroke();
		}
	}

	public void popStroke()
	{
		assert(sizec > 0);
		pointc -= sizev[--sizec]*2;
	}
}
