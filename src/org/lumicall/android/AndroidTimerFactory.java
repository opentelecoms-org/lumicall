package org.lumicall.android;

import java.util.concurrent.TimeUnit;

import org.zoolu.tools.InnerTimer;
import org.zoolu.tools.InnerTimerFactory;
import org.zoolu.tools.InnerTimerListener;

import android.content.Context;

public class AndroidTimerFactory implements InnerTimerFactory {
	
	Context context;
	
	public AndroidTimerFactory(Context context) {
		this.context = context;
	}

	@Override
	public InnerTimer createInnerTimer(long timeout, InnerTimerListener listener) {
		return new AndroidTimer(context, timeout, listener, TimeUnit.MILLISECONDS);
	}

}
