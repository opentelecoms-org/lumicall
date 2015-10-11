package org.lumicall.android.ganglia;

import java.util.logging.Logger;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import info.ganglia.gmetric4j.GSampler;
import info.ganglia.gmetric4j.Publisher;
import info.ganglia.gmetric4j.gmetric.GMetricSlope;
import info.ganglia.gmetric4j.gmetric.GMetricType;

public class WifiSampler extends GSampler {

	private static Logger log =
	        Logger.getLogger(WifiSampler.class.getName());
	private Context context;
	
	public WifiSampler(Context context, int interval) {
		super(0, interval, "lumicall");
		this.context = context;
	}
	
	protected void publishWifi(Publisher p) throws Exception {
		WifiManager wm = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
		WifiInfo wi = wm.getConnectionInfo();
		double rssi = Double.NaN;
		if(wi.getBSSID() != null) {
			rssi = wi.getRssi();
		}
		p.publish(process, "wifi_rssi",
				Double.toString(rssi), GMetricType.FLOAT, GMetricSlope.BOTH, "");
	}
	
	@Override
	public void run() {
		Publisher gm = getPublisher();
        log.finer("Announcing Android phone/Lumicall Wifi metrics");
        try {

			
			publishWifi(gm);
			
		} catch (Exception e) {
			log.severe("Exception while sending a metric");
			e.printStackTrace();
		}
	}


}
