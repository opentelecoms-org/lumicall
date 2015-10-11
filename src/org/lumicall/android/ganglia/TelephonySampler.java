package org.lumicall.android.ganglia;

import java.util.logging.Logger;

import android.content.Context;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import info.ganglia.gmetric4j.GSampler;
import info.ganglia.gmetric4j.Publisher;
import info.ganglia.gmetric4j.gmetric.GMetricSlope;
import info.ganglia.gmetric4j.gmetric.GMetricType;

public class TelephonySampler extends GSampler {
	
	private static Logger log =
	        Logger.getLogger(TelephonySampler.class.getName());
	
	private volatile double gsmSS = Double.NaN;
	
	TelephonyManager tm;

	public TelephonySampler(Context context, int interval) {
		super(0, interval, "lumicall");
		
		tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
		tm.listen(new GSMListener(), PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
	}
	
	class GSMListener extends PhoneStateListener {
		public void onSignalStrengthsChanged(SignalStrength ss) {
			gsmSS = ss.getGsmSignalStrength();
			if(gsmSS > 32)
				gsmSS = Double.NaN;
		}
	}

	protected void publishGSM(Publisher p) throws Exception {
		
		if(tm != null) {
			String deviceId = tm.getDeviceId();
			if(deviceId != null)
				p.publish(process, "gsm_imei",
						deviceId, GMetricType.STRING, GMetricSlope.BOTH, "");
			
			p.publish(process, "gsm_signal_strength",
						Double.toString(gsmSS), GMetricType.FLOAT, GMetricSlope.BOTH, "");
			
		}
	}

	@Override
	public void run() {
		Publisher gm = getPublisher();
        log.finer("Announcing Android phone/Lumicall metrics");
        try {
			
			publishGSM(gm);
			
		} catch (Exception e) {
			log.severe("Exception while sending a metric");
			e.printStackTrace();
		}
	}

}
