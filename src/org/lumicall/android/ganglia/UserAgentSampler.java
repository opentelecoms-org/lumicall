package org.lumicall.android.ganglia;

import java.util.logging.Logger;

import org.sipdroid.media.RtpStreamReceiver;
import org.sipdroid.media.RtpStreamSender;
import org.sipdroid.sipua.UserAgent;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;

import android.content.Context;

import info.ganglia.gmetric4j.GSampler;
import info.ganglia.gmetric4j.Publisher;
import info.ganglia.gmetric4j.gmetric.GMetricSlope;
import info.ganglia.gmetric4j.gmetric.GMetricType;

public class UserAgentSampler extends GSampler {
	
	private static Logger log =
	        Logger.getLogger(UserAgentSampler.class.getName());
	private Context context;

	public UserAgentSampler(Context context, int interval) {
		super(0, interval, "lumicall");
		this.context = context;
	}

	@Override
	public void run() {
		Publisher gm = getPublisher();
        log.finer("Announcing Android phone/Lumicall metrics");
        try {
			gm.publish(process, "instance_id", 
					Settings.getSIPInstanceId(context), GMetricType.STRING, GMetricSlope.BOTH, "");
			
			float loss_pct = Float.NaN;
			float lost_pct = Float.NaN;
			float late_pct = Float.NaN;
			float late_delay = Float.NaN;
			if(Receiver.call_state == UserAgent.UA_STATE_INCALL) {
				loss_pct = 0.0f;
				lost_pct = 0.0f;
				late_pct = 0.0f;
				late_delay = 0.0f;
				
				if (RtpStreamReceiver.good != 0) {
    				if (RtpStreamReceiver.timeout != 0) {
    					// No data
    				} else if (RtpStreamSender.m == 2) {
	    				loss_pct = Math.round(RtpStreamReceiver.loss/RtpStreamReceiver.good*100);
    					lost_pct = Math.round(RtpStreamReceiver.lost/RtpStreamReceiver.good*100);
	    				late_pct = Math.round(RtpStreamReceiver.late/RtpStreamReceiver.good*100);
	    				late_delay = ((RtpStreamReceiver.jitter-250*RtpStreamReceiver.mu)/8/RtpStreamReceiver.mu)/1000.0f;
    				} else {
    					lost_pct = Math.round(RtpStreamReceiver.lost/RtpStreamReceiver.good*100);
	    				late_pct = Math.round(RtpStreamReceiver.late/RtpStreamReceiver.good*100);
	    				late_delay = ((RtpStreamReceiver.jitter-250*RtpStreamReceiver.mu)/8/RtpStreamReceiver.mu)/1000.0f;
    				}
    				
    			}
    				
			}
			gm.publish(process, "loss_pct", 
					loss_pct + "", GMetricType.FLOAT, GMetricSlope.BOTH, "%");
			gm.publish(process, "lost_pct", 
					lost_pct + "", GMetricType.FLOAT, GMetricSlope.BOTH, "%");
			gm.publish(process, "late_pct", 
					late_pct + "", GMetricType.FLOAT, GMetricSlope.BOTH, "%");
			gm.publish(process, "late_delay", 
					late_delay + "", GMetricType.FLOAT, GMetricSlope.BOTH, "s");
			
		} catch (Exception e) {
			log.severe("Exception while sending a metric");
			e.printStackTrace();
		}
	}

}
