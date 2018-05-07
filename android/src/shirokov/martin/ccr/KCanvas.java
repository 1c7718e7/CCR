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

import shirokov.martin.ccr.Worker;

import android.view.View;
import android.view.MotionEvent;
import android.graphics.*;
import android.content.Context;
import android.widget.Toast;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class KCanvas extends View {
	protected final int BG_COLOR = 0xFF202020;
	protected final int FG_COLOR = 0xFFFFFFFF;
	protected final int BAD_COLOR = 0xFFFF2000;
	protected Paint mPaint, mPaintOut, mPaintArrow;
	protected int mSize;
	protected Kanji mKanji;
	protected float mPrevX, mPrevY;
	protected List<Listener> mListeners;
	protected Worker.Feedback mFeedback;
	protected float[] mOrderArrows;

	public interface Listener {
		public void onNewStroke();
	};

	public KCanvas(Context ctx)
	{
		super(ctx);
		mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaint.setColor(FG_COLOR);
		mPaint.setStrokeWidth(.02f);
		mPaint.setStrokeCap(Paint.Cap.ROUND);
		mPaintOut = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaintOut.setColor(0xFF000000);
		mPaintOut.setStrokeWidth(.03f);
		mPaintOut.setStrokeCap(Paint.Cap.ROUND);
		mPaintArrow = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaintArrow.setColor(0xff3080ff);
		mPaintArrow.setStrokeWidth(.005f);
		mPaintOut.setStrokeCap(Paint.Cap.ROUND);
		mKanji = new Kanji();
		mListeners = new ArrayList<Listener>();
	}

	public void setFeedback(Worker.Feedback b)
	{
		assert(b != null);
		invalidate();
		mFeedback = b;
		/*
		if (b.user_map.length == b.model_map.length) {
			int errc = 0;
			for (int i = 0; i < b.kanji.sizec; i++) {
				int j = b.model_map[i];
				if (j > 1) {
					if (b.user_map[j-1]+1 != b.user_map[j])
						errc++;
				}
			}
			float[] ars = new float[4*errc];
			errc = 0;
			int ofs = 0;
			for (int i = 0; i < b.kanji.sizec; i++) {
				int j = b.model_map[i];
				if (j > 1) {
					if (b.user_map[j-1]+1 != b.user_map[j]) {
						if (ofs == 0) {
							ars[4*errc+0] = 0;
							ars[4*errc+1] = 0;
						} else {
							ars[4*errc+0] = (float)b.kanji.pointv[ofs-2];
							ars[4*errc+1] = (float)b.kanji.pointv[ofs-1];
						}
						ars[4*errc+2] = (float)b.kanji.pointv[ofs+0];
						ars[4*errc+3] = (float)b.kanji.pointv[ofs+1];
						errc++;
					}
				}
				ofs += b.kanji.sizev[i]*2;
			}
			mOrderArrows = ars;
		}
		*/
		mKanji = mFeedback.kanji;
		for (int i = 0; i < mKanji.pointc; i++) {
			mKanji.pointv[i] *= .9f;
			mKanji.pointv[i] += .05f;
		}
	}

	public Kanji getKanji()
	{
		return mKanji.clone();
	}

	public void addListener(Listener l) { mListeners.add(l); }

	@Override
	public void onLayout(boolean changed, int x1, int y1, int x2, int y2)
	{
		int w = x2-x1;
		int h = y2-y1;
		mSize = Math.min(w, h);
	}

	private void drawArrow(Canvas c, double sx, double sy, double dx, double dy, Paint p)
	{
		double x, y, l;
		c.drawLine((float)sx, (float)sy, (float)dx, (float)dy, p);
		x = sx-dx;
		y = sy-dy;
		l = 1.f/Math.sqrt(x*x + y*y);
		x *= l;
		y *= l;
		double px = -y;
		double py = x;
		c.drawLine(
			(float)dx, (float)dy,
			(float)(dx + 0.03f*(px+x)), (float)(dy + 0.03f*(py+y)),
			p);
		c.drawLine(
			(float)dx, (float)dy,
			(float)(dx + 0.03f*(-px+x)), (float)(dy + 0.03f*(-py+y)),
			p);
	}

	private void drawStroke(Canvas c, double[] pv, int ofs, int n)
	{
		for (int j = 2; j < n*2; j += 2) {
			c.drawLine(
				(float)pv[ofs+j-2], (float)pv[ofs+j-1],
				(float)pv[ofs+j+0], (float)pv[ofs+j+1],
				mPaintOut);
		}
		for (int j = 2; j < n*2; j += 2) {
			c.drawLine(
				(float)pv[ofs+j-2], (float)pv[ofs+j-1],
				(float)pv[ofs+j+0], (float)pv[ofs+j+1],
				mPaint);
		}
	}

	private void drawKanji(Canvas c)
	{
		c.drawColor(BG_COLOR);
		c.save();
		c.scale(mSize, mSize, 0, 0);
		int ofs = 0;
		mPaint.setColor(FG_COLOR);
		for (int i = 0; i < mKanji.sizec; i++) {
			drawStroke(c, mKanji.pointv, ofs, mKanji.sizev[i]);
			ofs += mKanji.sizev[i]*2;
		}
		assert(ofs == mKanji.pointc);
		if (mOrderArrows != null) {
			/* draw order correcting arrows */
			mPaintArrow.setColor(BAD_COLOR);
			ofs = 0;
			for (int i = 0; i < mOrderArrows.length; i += 4) {
				drawArrow(c,
					(float)mOrderArrows[i+0], (float)mOrderArrows[i+1],
					(float)mOrderArrows[i+2], (float)mOrderArrows[i+3],
					mPaintArrow);
			}
		}
		/* draw missing strokes */
		mPaint.setColor(BAD_COLOR);
		if (mFeedback != null) {
			ofs = 0;
			for (int i = 0; i < mFeedback.kanji.sizec; i++) {
				if (mFeedback.model_map[i] == -1)
					drawStroke(c, mFeedback.kanji.pointv, ofs, mFeedback.kanji.sizev[i]);
				ofs += mFeedback.kanji.sizev[i]*2;
			}
		}
		c.restore();
	}

	private void drawCenteredText(Canvas c, Paint p, String s, float x, float y)
	{
		Rect bb = new Rect();
		p.getTextBounds(s, 0, s.length(), bb);
		x -= bb.centerX();
		y -= bb.centerY();
		c.drawText(s, x, y, p);
	}

	@Override
	public void onDraw(Canvas c)
	{
		if (mKanji.sizec == 0) {
			c.drawColor(BG_COLOR);
			mPaint.setColor(FG_COLOR);
			mPaint.setTextSize(mSize/8.f);
			drawCenteredText(c, mPaint, "Write here", mSize/2, mSize/2);
		} else {
			drawKanji(c);
			mPaint.setColor(FG_COLOR);
			mPaint.setTextSize(mSize/20.f);
			drawCenteredText(c, mPaint, "Click on a search result to copy it", mSize/2, mSize*0.95f);
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent ev)
	{
		if (mFeedback != null)
			clear();
		switch (ev.getActionMasked()) {
		case MotionEvent.ACTION_MOVE:
			int hsize = ev.getHistorySize();
			for (int t = 0; t < hsize; t++) {
				float x = ev.getHistoricalX(0, t);
				float y = ev.getHistoricalY(0, t);
				float d = (mPrevX-x)*(mPrevX-x) + (mPrevY-y)*(mPrevY-y);
				if (d < 4)
					continue;
				mKanji.pushPoint(x/mSize, y/mSize);
				mPrevX = x;
				mPrevY = y;
			}
			invalidate();
			return true;
		case MotionEvent.ACTION_DOWN:
			mKanji.pushStroke();
			mPrevX = ev.getX();
			mPrevY = ev.getY();
			mKanji.pushPoint(mPrevX/mSize, mPrevY/mSize);
			invalidate();
			return true;
		case MotionEvent.ACTION_UP:
			mPrevX = ev.getX();
			mPrevY = ev.getY();
			mKanji.pushPoint(mPrevX/mSize, mPrevY/mSize);
			mKanji.endStroke();
			for (Listener l : mListeners)
				l.onNewStroke();
			invalidate();
			return true;
		case MotionEvent.ACTION_CANCEL:
			mKanji.popStroke();
			invalidate();
			return true;
		}
		return true;
	}

	public void clear()
	{
		while (undo())
			;
	}

	public boolean undo()
	{
		if (mKanji.sizec == 0)
			return false;
		mKanji.popStroke();
		invalidate();
		mFeedback = null;
		mOrderArrows = null;
		return true;
	}
}
