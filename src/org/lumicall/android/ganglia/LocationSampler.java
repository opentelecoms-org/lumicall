package org.lumicall.android.ganglia;

import java.util.logging.Logger;

import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.Bundle;
import android.os.PowerManager;
import info.ganglia.gmetric4j.GSampler;
import info.ganglia.gmetric4j.Publisher;
import info.ganglia.gmetric4j.gmetric.GMetricSlope;
import info.ganglia.gmetric4j.gmetric.GMetricType;

public class LocationSampler extends GSampler {
	
	private static Logger log =
	        Logger.getLogger(LocationSampler.class.getName());
	
	volatile Location my_location = null;
	
	PowerManager.WakeLock wl;

	public LocationSampler(Context context, int interval) {
		super(0, interval, "lumicall");
		
		LocationListener locationListener = new MyLocationListener();
		LocationManager lm = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
		lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 35000, 10, locationListener);
		
		PowerManager pm = (PowerManager)context.getSystemService(
                Context.POWER_SERVICE);
		wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LocationSampler.class.getName());
		wl.acquire();
	}
	
	private final class MyLocationListener implements LocationListener {

        @Override
        public void onLocationChanged(Location locFromGps) {
            my_location = locFromGps;
        }

        @Override
        public void onProviderDisabled(String provider) {
           my_location = null;
        }

        @Override
        public void onProviderEnabled(String provider) {
           // called when the GPS provider is turned on (user turning on the GPS on the phone)
        }

		@Override
		public void onStatusChanged(String provider, int status, Bundle extras) {
			if(status != LocationProvider.AVAILABLE)
				my_location = null;
		}
	}
	
	protected void publishLocation(Publisher p) throws Exception {
		
		Location _location = my_location;
		if(_location == null)
			return;
		
		p.publish(process, "location_latitude",
				Double.toString(_location.getLatitude()), GMetricType.FLOAT, GMetricSlope.BOTH, "");
		p.publish(process, "location_longitude",
				Double.toString(_location.getLongitude()), GMetricType.FLOAT, GMetricSlope.BOTH, "");
		p.publish(process, "location_altitude",
				Double.toString(_location.getAltitude()), GMetricType.FLOAT, GMetricSlope.BOTH, "");
		if(_location.hasSpeed())
			p.publish(process, "location_speed",
					Double.toString(_location.getSpeed()), GMetricType.FLOAT, GMetricSlope.BOTH, "");
	}

	@Override
	public void run() {
		Publisher gm = getPublisher();
        log.finer("Announcing Android phone/Lumicall Location metrics (if available");
        try {
			
			publishLocation(gm);
			
		} catch (Exception e) {
			log.severe("Exception while sending a metric");
			e.printStackTrace();
		}
	}


}
