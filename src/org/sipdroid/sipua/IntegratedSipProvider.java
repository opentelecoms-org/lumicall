package org.sipdroid.sipua;

import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Sipdroid;
import org.zoolu.sip.message.Message;
import org.zoolu.sip.provider.SipProvider;
import org.zoolu.sip.provider.Transport;

import android.content.Context;
import android.os.PowerManager;

public class IntegratedSipProvider extends SipProvider {

	public IntegratedSipProvider(String localIpAddress, int i) {
		super(localIpAddress, i);
		
	}
	
	PowerManager pm;
	PowerManager.WakeLock wl;
	
	/** When a new SIP message is received. */
	public void onReceivedMessage(Transport transport, Message msg) {
		if (pm == null) {
			pm = (PowerManager) Receiver.mContext.getSystemService(Context.POWER_SERVICE);
			wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Sipdroid.SipProvider");
		}
		wl.acquire(); // modified
		super.onReceivedMessage(transport, msg);
		wl.release();
	}

	/** When Transport terminates. */
	public void onTransportTerminated(Transport transport, Exception error) {
		super.onTransportTerminated(transport, error);
		if (transport.getProtocol().equals(PROTO_TCP) ||
				transport.getProtocol().equals(PROTO_TLS)) {
			if (Sipdroid.on(Receiver.mContext))
				Receiver.engine(Receiver.mContext).register(); // modified
		}
	}
	
	public void onReconnected(Message msg) {
		if (!msg.isRegister())
			Receiver.engine(Receiver.mContext).register(); // modified
	}
	
}
