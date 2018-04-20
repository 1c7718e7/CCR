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

import android.content.Context;
import android.app.Activity;
import android.widget.TextView;
import android.view.ViewGroup;
import android.view.Window;
import android.os.Bundle;
import android.widget.Toast;
import android.text.ClipboardManager;
import android.util.Log;

import java.util.List;

public class CCRActivity extends Activity
                         implements KMenu.Listener, KCanvas.Listener, Worker.Callback
{
	private class Layout extends ViewGroup {
		protected KCanvas mCanvas;
		protected KMenu mMenu;
		protected int mW, mH, mCSize;

		public Layout(CCRActivity a)
		{
			super(a);

			mCanvas = new KCanvas(a);
			addView(mCanvas);
			mCanvas.addListener(a);

			mMenu = new KMenu(a);
			addView(mMenu);
			mMenu.addListener(a);
		}

		@Override
		protected void onLayout(boolean changed, int x1, int y1, int x2, int y2)
		{
			mW = x2-x1;
			mH = y2-y1;
			boolean hor = mW > mH;
			mCSize = Math.min(mW, mH);
			int other = Math.max(mW, mH);
			int maxCSize = other*3/4;
			mCSize = Math.min(maxCSize, mCSize); // reserve some space for the menu
			mCanvas.layout(x1, y1, x1+mCSize, y1+mCSize);
			if (hor) {
				mMenu.layout(x1+mCSize, y1, x2, y2);
			} else {
				mMenu.layout(x1, y1+mCSize, x2, y2);
			}
		}
	};
	private Layout mLayout;
	private Worker mWorker;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		mLayout = new Layout(this);
		setContentView(mLayout);
		mWorker = new Worker(this);
	}

	@Override
	public void onDestroy()
	{
		mWorker.interrupt();
		super.onDestroy();
	}

	@Override
	public void onSelect(Match i)
	{
		ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE); 
		clipboard.setText(String.valueOf((char)i.code));
		Worker.Feedback f = mWorker.getFeedback(mLayout.mCanvas.getKanji(), i.cookie);
		mLayout.mCanvas.setFeedback(f);
	}

	@Override
	public void onResult(final Kanji k, final Match[] res)
	{
		// we are in the Worker thread here
		runOnUiThread(new Runnable() {
			public void run()
			{
				mLayout.mMenu.setItems(res);
				setProgressBarIndeterminateVisibility(false);
			}
		});
	}

	@Override
	public void onNewStroke()
	{
		mWorker.setQuery(new Worker.Query(mLayout.mCanvas.getKanji(), this));
		setProgressBarIndeterminateVisibility(true);
	}

	@Override
	public void onBackPressed() {
		if (!mLayout.mCanvas.undo())
			finish();
		onNewStroke();
	}
}
