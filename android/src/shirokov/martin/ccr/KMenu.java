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

import android.view.View;
import android.view.MotionEvent;
import android.graphics.*;
import android.content.Context;
import android.widget.Toast;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class KMenu extends View {
	public interface Listener {
		void onSelect(Match i);
	};
	protected int mW, mH;
	protected Match[] mItems = null;
	protected int mSelected = -1;
	protected List<Listener> mListeners = new ArrayList<Listener>();
	protected Paint mPaint, mPaintSel;
	protected int mSquareSize, mLineLen;

	public KMenu(Context ctx)
	{
		super(ctx);
		mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaint.setColor(0xFFFFFFFF);
		mPaintSel = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaintSel.setColor(0x5000FF30);
		setItems(null);
	}

	public void setItems(Match[] i)
	{
		if (i == null)
			i = new Match[] {};
		mItems = i;
		mSelected = -1;
		calcPositions();
	}

	public Match getSelected() { return mItems[mSelected]; }

	public void addListener(Listener l) { mListeners.add(l); }

	@Override
	public void onLayout(boolean changed, int x1, int y1, int x2, int y2)
	{
		mW = x2-x1;
		mH = y2-y1;
		calcPositions();
	}

	protected void calcPositions()
	{
		float l = 0, r = Math.min(mW, mH);
		while (l+0.1 < r) {
			float m = (l+r)/2;
			int len = (int)(mW/m);
			int linec = (int)(mH/m);
			//Log.i("ccr.KMenu", "size "+m+" => "+len+"*"+linec+" = "+(len*linec));
			if (len*linec >= mItems.length)
				l = m;
			else
				r = m;
		}
		mSquareSize = (int)l;
		if (mSquareSize == 0)
			mSquareSize = 1; 
		mLineLen = (int)(mW/mSquareSize);
		//Log.i("ccr.KMenu", "size "+mSquareSize+"; len "+mLineLen);
		mPaint.setTextSize(mSquareSize);
		invalidate();
	}

	@Override
	public void onDraw(Canvas c)
	{
		c.drawColor(0);
		float y = 0;
		int i = 0;
		drawItems: while (true) {
			float x = 0;
			for (int j = 0; j < mLineLen; j++) {
				if (i >= mItems.length)
					break drawItems;
				//Log.i("ccr.KMenu", "item "+i+" at "+x+","+y);
				if (mSelected == i)
					c.drawRect(x, y, x+mSquareSize, y+mSquareSize, mPaintSel);
				c.drawText(String.valueOf((char)mItems[i].code), x, y+mSquareSize-mPaint.descent()/2, mPaint);
				x += mSquareSize;
				i++;
			}
			y += mSquareSize;
		}
	}

	private void setSelected(int i)
	{
		mSelected = i;
		invalidate();
		for (Listener l : mListeners)
			l.onSelect(mItems[mSelected]);
	}

	@Override
	public boolean onTouchEvent(MotionEvent ev)
	{
		switch (ev.getActionMasked()) {
		case MotionEvent.ACTION_UP:
			int i = (int)(ev.getX()/mSquareSize) + (int)(ev.getY()/mSquareSize)*mLineLen;
			//Log.i("ccr.KMenu", "click at "+ev.getX()+","+ev.getY()+" => index "+i);
			if (i < mItems.length)
				setSelected(i);
			return true;
		}
		return true;
	}
}
