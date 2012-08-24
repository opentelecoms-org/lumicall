package org.lumicall.android.ganglia;

import java.util.UUID;
import java.util.logging.Logger;

import org.sipdroid.sipua.ui.Settings;

import ganglia.CoreSampler;
import ganglia.GMonitor;
import ganglia.gmetric.GMetric;
import ganglia.gmetric.GMetric.UDPAddressingMode;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.preference.PreferenceManager;

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
        	
        	SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        	
        	// Is Ganglia monitoring enabled?
        	if(!sp.getBoolean(Settings.PREF_GANGLIA_ENABLE, Settings.DEFAULT_GANGLIA_ENABLE))
        		return;
        	
        	UUID uuid = null;
        	if(sp.getBoolean(Settings.PREF_GANGLIA_UUID_ENABLE, Settings.DEFAULT_GANGLIA_UUID_ENABLE)) {
        		uuid = UUID.fromString(Settings.getSIPInstanceId(this));
        	}
        	
            a = new GMonitor(new AndroidGScheduler(this));
            String dest = sp.getString(Settings.PREF_GANGLIA_DEST, Settings.DEFAULT_GANGLIA_DEST);
            int destPort = Settings.getStringAsInt(sp, Settings.PREF_GANGLIA_PORT, Settings.DEFAULT_GANGLIA_PORT);
            int ttl = Settings.getStringAsInt(sp, Settings.PREF_GANGLIA_TTL, Settings.DEFAULT_GANGLIA_TTL);
            a.setGmetric(new GMetric(dest, destPort, UDPAddressingMode.getModeForAddress(dest), ttl,
            		true, uuid));
            
            // Is heartbeat sending required?
            if(!sp.getBoolean(Settings.PREF_GANGLIA_HEARTBEAT, Settings.DEFAULT_GANGLIA_HEARTBEAT)) {
            	log.info("will send heartbeat");
            	a.addSampler(new CoreSampler());
            }
            
            int interval = Settings.getStringAsInt(sp, Settings.PREF_GANGLIA_INTERVAL, Settings.DEFAULT_GANGLIA_INTERVAL);
            
            a.addSampler(new UserAgentSampler(this, interval));
            a.addSampler(new WifiSampler(this, interval));
            a.addSampler(new TelephonySampler(this, interval));
            a.addSampler(new BatterySampler(this, interval));
            
            a.start();

            log.info("GMonitorService started");
        } catch ( Exception ex ) {
            log.severe("Exception starting GMonitor");
            ex.printStackTrace();
        }
		
	}
	
}
