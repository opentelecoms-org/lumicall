package org.lumicall.android;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

import org.zoolu.tools.InnerTimer;
import org.zoolu.tools.InnerTimerListener;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.SystemClock;

public class AndroidTimer extends BroadcastReceiver implements InnerTimer {
	
	private static Logger log =
		Logger.getLogger(AndroidTimer.class.getName());
	
	private final static String URI_SCHEME = "siptimer";
	static volatile Map<UUID, AndroidTimer> timers = new HashMap<UUID,AndroidTimer>();

	private InnerTimerListener listener = null;
	
	// This constructed is invoked when used as a BroadcastReceiver
	public AndroidTimer() {
		
	}

	public AndroidTimer(Context context, long timeout, InnerTimerListener listener,
			TimeUnit unit) {
		
		this.listener = listener;
		
		AlarmManager am = (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);
        Intent _intent = new Intent(context, AndroidTimer.class);
        
        UUID timerId = UUID.randomUUID();
        Uri data = Uri.parse(URI_SCHEME + ":" + timerId);
        _intent.setData(data);
        
        synchronized(timers) {
        	timers.put(timerId, this);
        }
        
        am.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, 
        		SystemClock.elapsedRealtime() + unit.toMillis(timeout),
        		PendingIntent.getBroadcast(context, 0, _intent, 0));
		
        log.info("created an AndroidTimer for " + timeout + " " + unit.name() + ", id = " + data);
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		Uri data = intent.getData();
		log.info("onReceive: " + data);
		
		if(data == null)
			return;
		
		if(!data.getScheme().equals(URI_SCHEME))
			return;
		
		log.info("it's a timeout");
		
		try {
			UUID timerId = UUID.fromString(data.getEncodedSchemeSpecificPart());
			AndroidTimer t = null;
			synchronized(timers) {
				t = timers.remove(timerId);
			}
			if(t != null)
				t.onTimeout();
			else
				log.severe("timer lookup failed");
		} catch (Exception ex) {
			log.severe("timer lookup or callback failed: " + ex.getMessage());
			ex.printStackTrace();
		}
	}

	private void onTimeout() {
		listener.onInnerTimeout();
	}
}
