package org.lumicall.android.ganglia;

import java.util.Vector;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import info.ganglia.gmetric4j.GScheduler;

public class AndroidGScheduler extends BroadcastReceiver implements GScheduler {
	
	private static Logger log =
			Logger.getLogger(AndroidGScheduler.class.getName());
	
	private Context context;
	static Vector<Runnable> tasks = new Vector<Runnable>();
	
	private final static String URI_SCHEME = "gtask";
	
	private final static String INTENT_NAME = "org.lumicall.android.ganglia.intent.ALARM_EVENT";
	
	private final static int INITIAL_DELAY = 2000;
	
	public AndroidGScheduler() {
		this.context = null;
		log.info("scheduler created (default constructor)");
	}

	public AndroidGScheduler(Context context) {
		this.context = context;
		log.info("scheduler created (with context)");
	}

	@Override
	public void onStart() {
		
	}

	@Override
	public void onStop() {
		
	}

	@Override
	public void scheduleAtFixedRate(Runnable command, long initialDelay,
			long period, TimeUnit unit) {
		
		tasks.add(command);
		int i = tasks.size() - 1;
		
		log.info("trying to set up a scheduled event, i = " + i);
		
		AlarmManager am = (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);
        Intent _intent = new Intent(context, AndroidGScheduler.class);
        Uri data = Uri.parse(URI_SCHEME + ":" + i);
        _intent.setData(data);
        am.setRepeating(AlarmManager.ELAPSED_REALTIME_WAKEUP, 
        		INITIAL_DELAY + unit.toMillis(initialDelay),
        		unit.toMillis(period),
        		PendingIntent.getBroadcast(context, 0, _intent, 0));
        
        log.info("done setting up a scheduled event: " + data + ", i = " + i);
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		Uri data = intent.getData();
		log.info("scheduler onReceive: " + data);
		
		if(data == null)
			return;
		
		if(!data.getScheme().equals(URI_SCHEME))
			return;
		
		try {
			int i = Integer.parseInt(data.getEncodedSchemeSpecificPart());
			Runnable r = tasks.get(i);
			r.run();
		} catch (Exception ex) {
			log.severe("task failed: " + ex.getMessage());
			ex.printStackTrace();
		}
	}

}
