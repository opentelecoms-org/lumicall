package org.lumicall.android.ganglia;

import java.util.logging.Logger;

import ganglia.CoreSampler;
import ganglia.GMonitor;
import ganglia.gmetric.GMetric;
import ganglia.gmetric.GMetric.UDPAddressingMode;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class GMonitorService extends Service {
	
	private static Logger log =
		Logger.getLogger(GMonitorService.class.getName());

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public void onCreate() {
		super.onCreate();
		log.info("onCreate called");
	}

	// This is the old onStart method that will be called on the pre-2.0
	// platform.  On 2.0 or later we override onStartCommand() so this
	// method will not be called.
	@Override
	public void onStart(Intent intent, int startId) {
		handleCommand(intent);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		handleCommand(intent);
		// We want this service to continue running until it is explicitly
		// stopped, so return sticky.
		return START_STICKY;
	}
	
	private void handleCommand(Intent intent) {
		
		GMonitor a = null ;
        try {
            a = new GMonitor();
            a.setGmetric(new GMetric("239.2.11.71", 8649, UDPAddressingMode.MULTICAST));
            a.addSampler(new CoreSampler());
            a.addSampler(new AndroidSampler(this));
            a.start();
            log.info("GMonitorService started");
        } catch ( Exception ex ) {
            log.severe("Exception starting GMonitor");
            ex.printStackTrace();
        }
		
	}
	
}
