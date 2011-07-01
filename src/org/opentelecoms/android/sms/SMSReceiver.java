package org.opentelecoms.android.sms;

import java.io.StringWriter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.opentelecoms.android.reg.EnrolmentService;
import org.opentelecoms.android.sip.RegistrationFailedException;
import org.opentelecoms.android.sip.RegistrationUtil;
import org.sipdroid.sipua.R;
import org.xmlpull.v1.XmlSerializer;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SmsMessage;
import android.util.Log;
import android.util.Xml;
import android.widget.Toast;

public class SMSReceiver extends BroadcastReceiver {
	private static final String LOG_TAG = "SIP-UA-SMS";

	static final String ACTION =
		"android.provider.Telephony.SMS_RECEIVED";

	static Pattern pattern = Pattern.compile("(\\*\\*\\p{Alnum}{6}\\*\\*)");

	public void onReceive(Context context, Intent intent) {
		try {
		if (intent.getAction().equals(ACTION)) {
			SmsMessage[] msgs;
			Bundle bundle = intent.getExtras();
			Object[] pdus = (Object[])bundle.get("pdus");
            msgs = new SmsMessage[pdus.length];
            //byte[] data = null;
            String info = "SMS msg: ";
            
            Log.v(LOG_TAG, "rx msgs: " + msgs.length);
            
            for (int i=0; i<msgs.length; i++) {
                msgs[i] = SmsMessage.createFromPdu((byte[])pdus[i]);                
                // msgs[i].getOriginatingAddress();                     
                //data = msgs[i].getUserData();
                
                /*for(int index=0; index<data.length; ++index) {                
                        info += Character.toString((char)data[index]);
                }*/
                info += msgs[i].getMessageBody();
            }

            // Toast.makeText(context, info, Toast.LENGTH_SHORT).show();

            Matcher m = pattern.matcher(info);
            if(m.find()) {
            	String regCode = m.group(1);
            	handleRegistrationCode(context, regCode);
            }
			
			/* NotificationManager nm = (NotificationManager) context.getSystemService(
					Context.NOTIFICATION_SERVICE);

			int icon = R.drawable.icon22;
			Notification n = new Notification(icon, info, System.currentTimeMillis());
			nm.notify(123, n); */
					

		}
		} catch (Exception ex) {
			Log.e(LOG_TAG, ex.toString());
		}
	}
	
	protected void handleRegistrationCode(Context context, String regCode) {
		// Send a new intent
		
		final Intent intent = new Intent(context, EnrolmentService.class);
		intent.setAction(EnrolmentService.ACTION_VALIDATE);
		//intent.setAction(RegistrationPhaseTwo.ACTION);
		intent.putExtra(EnrolmentService.VALIDATION_CODE, regCode);
		context.startService(intent);
	}


}
