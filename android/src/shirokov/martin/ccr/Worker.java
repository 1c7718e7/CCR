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

import shirokov.martin.ccr.*;

import android.util.Log;
import android.util.Log;
import android.content.res.AssetManager;
import android.content.Context;

import java.io.*;
import java.util.Arrays;

public class Worker implements Runnable
{
	private static class CCR
	{
		static {
			System.loadLibrary("ccr");
			initJNI();
		}
		/* AFAIK the jre uses longs for native pointers too */
		public static native void initJNI();
		public static native void finiJNI();
		public static native long open(byte[] data, long size);
		public static native void close(long db);
		public static native Match[] lookup(long db, Kanji k, int maxres);
		//public static native int[] feedback(long db, long cookie,
		//	double[] pointv, int[] sizev, int sizec, int ui[]);
		public static native Feedback feedback(long db, Kanji u, Match.Cookie m);
	}

	private final int MAXRESULTS = 16;
	public interface Callback {
		public void onResult(Kanji k, Match[] res);
	};

	public static class Query {
		private Kanji kanji;
		private Callback callback;
		public Query(Kanji k, Callback c)
		{
			kanji = k;
			callback = c;
		}
	};

	private Query mQuery;
	private Thread mThread;
	private Context mCtx;
	private long mDB;

	public static class Feedback {
		int[] user_map;
		int[] model_map;
		Kanji kanji;
		public Feedback(int[] u, int[] m, Kanji k) { user_map = u; model_map = m; kanji = k; }
	};

	public Worker(Context ctx)
	{
		mQuery = null;
		mThread = new Thread(this);
		mThread.start();
		mCtx = ctx;
	}

	// it's not possible to (correctly) call this before db is opened,
	// because 'model' has to come from a query
	public Feedback getFeedback(Kanji user, Match.Cookie model)
	{
		assert(mDB != 0);
		return CCR.feedback(mDB, user, model);
	}

	public void interrupt() { mThread.interrupt(); }

	// We do not do any queueing. Canceling the previous query is what the ui needs.
	public synchronized void setQuery(Query q)
	{
		assert(q != null);
		mQuery = q;
		notify();
	}

	private synchronized Query getQuery()
	{
		while (mQuery == null) {
			//Log.i("ccr.Worker", "waiting for a query...");
			try { wait(); }
			catch (InterruptedException e) { return null; }
			//Log.i("ccr.Worker", "woken up!");
		}
		return mQuery;
	}

	private synchronized boolean checkQuery(Query q)
	{
		if (mQuery == q) {
			//Log.i("ccr.Worker", "query still actual; submiting");
			mQuery = null;
			return true;
		} else {
			//Log.i("ccr.Worker", "query outdated");
			return false;
		}
	}

	private long openDB() throws IOException
	{
		InputStream i = mCtx.getAssets().open("kanji.list");
		int n = 0;
		byte[] buffer = new byte[8*1024*1024];
		while (true) {
			n += i.read(buffer, n, buffer.length-n);
			if (n == buffer.length) {
				byte[] b = new byte[buffer.length*2];
				for (int j = 0; j < n; j++)
					b[j] = buffer[j];
				buffer = b;
			} else
				break;
		}
		i.close();
		long d = CCR.open(buffer, n);
		//Log.i("ccr.Worker", "db pointer = "+d);
		return d;
	}

	public void run()
	{
		Query q;

		try {
			mDB = openDB();
		} catch (IOException e) {
			throw new RuntimeException(e); // if reading an internal asset failed...
		}
		//Log.i("ccr.Worker", "worker started");
		while (true) {
			q = getQuery();
			if (q == null)
				break;
			Match[] res = CCR.lookup(mDB, q.kanji, MAXRESULTS);
			if (checkQuery(q))
				q.callback.onResult(q.kanji, res);
		}
		CCR.close(mDB);
	}
}
