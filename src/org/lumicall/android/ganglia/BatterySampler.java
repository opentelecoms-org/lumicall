package org.lumicall.android.ganglia;

import java.util.logging.Logger;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import info.ganglia.gmetric4j.GSampler;
import info.ganglia.gmetric4j.Publisher;
import info.ganglia.gmetric4j.gmetric.GMetricSlope;
import info.ganglia.gmetric4j.gmetric.GMetricType;

public class BatterySampler extends GSampler {
	
	private static Logger log =
	        Logger.getLogger(BatterySampler.class.getName());
	private Context context;
	
	public BatterySampler(Context context, int interval) {
		super(0, interval, "lumicall");
		this.context = context;
	}

	protected void publishBattery(Publisher p) throws Exception {
		IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
		Intent batteryStatus = context.registerReceiver(null, ifilter);
		// Are we charging / charged?
		int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
		boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING ||
		                     status == BatteryManager.BATTERY_STATUS_FULL;

		// How are we charging?
		int chargePlug = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
		//boolean usbCharge = chargePlug == BATTERY_PLUGGED_USB;
		//boolean acCharge = chargePlug == BATTERY_PLUGGED_AC;
		
		int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
		int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);

		double batteryPct = level / (float)scale;
		
		p.publish(process, "battery_pct",
				Double.toString(batteryPct), GMetricType.FLOAT, GMetricSlope.BOTH, "");
	}
	
	@Override
	public void run() {
		Publisher gm = getPublisher();
        log.finer("Announcing Android phone/Lumicall Battery metrics");
        try {
			
        	publishBattery(gm);
			
		} catch (Exception e) {
			log.severe("Exception while sending a metric");
			e.printStackTrace();
		}
	}

}
