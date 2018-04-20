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

public class Match {
	public static class Cookie {
		/* This class is deliberately a black box to java code;
		   it's used only through jni */
		private long ptr;
		private Cookie(long p) { ptr = p; }
	};
	public double score;
	public int code;
	public Cookie cookie;
	public Match(double s, int c, Cookie ck)
	{
		score = s;
		code = c;
		cookie = ck;
	}
};
