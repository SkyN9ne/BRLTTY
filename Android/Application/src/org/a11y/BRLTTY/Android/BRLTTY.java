/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2012 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

package org.a11y.BRLTTY.Android;

import org.a11y.BRLTTY.Core.*;

import android.util.Log;

import android.app.Activity;
import android.os.Bundle;

import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class BRLTTY extends Activity {
  private final String LOG_TAG = this.getClass().getName();

  private Thread coreThread = null;
  private Button stopButton;

  @Override
  public void onCreate (Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);

    stopButton = (Button)findViewById(R.id.stop);
    stopButton.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick (View view) {
        Log.d(LOG_TAG, "stop button pressed");
        Wrapper.stop = true;
      }
    });

    coreThread = new CoreThread(this);
    coreThread.start();
  }
}
