package org.opentelecoms.android.reg;

import java.io.StringWriter;

import org.opentelecoms.android.sip.RegisterAccount;
import org.opentelecoms.android.sip.RegistrationFailedException;
import org.opentelecoms.android.sip.RegistrationUtil;
import org.sipdroid.sipua.ui.Settings;
import org.xmlpull.v1.XmlSerializer;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.util.Log;
import android.util.Xml;

public class RegistrationPhaseTwo extends BroadcastReceiver {

	public static final String ACTION = "org.opentelecoms.intent.HANDLE_AUTH_CODE";
	
	private static final String DEFAULT_SIP_SERVER = "sip.readytechnology.co.uk";
	
	public static final String REG_CODE = "regCode";
	
	private static final String LOG_TAG = "RegPhase2";
	
	public void onReceive(Context context, Intent intent) {
		
		if (intent.getAction().equals(ACTION)) {
			
			// Get the reg code from the SMS
			String regCode = intent.getStringExtra(REG_CODE);

			handleRegistrationCode(context, regCode);
			
			setupSIP(context);
		}
	}
	
	
	protected String getBodyXml(String regCode) {

		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "activation");
		
			RegistrationUtil.serializeProperty(serializer, "regCode", regCode);
			
			serializer.endTag("", "activation");
			serializer.endDocument();
			return writer.toString();
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
	}
	
	protected String getEncryptedXml(Context context, String s) {
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "encryptedActivation");
			//serializer.attribute("", "regNum", getRegNum());

			String encryptedText = RegistrationUtil.getEncryptedStringAsBase64(context, s);
			serializer.text(RegistrationUtil.getEncryptedStringAsBase64(context, s));

			serializer.endTag("", "encryptedActivation");
			serializer.endDocument();
			//return writer.toString();
			return encryptedText;
		} catch (Exception e) {
			Log.e(LOG_TAG, e.toString());
			return null;
		}
	}	
	
	
	
	protected void handleRegistrationCode(final Context context, final String regCode) {
		(new Thread() {
			public void run() {
				
				try {
					
				
					String s = getBodyXml(regCode);
					RegistrationUtil.submitMessage(getEncryptedXml(context, s));  

				    
				
				} catch (RegistrationFailedException e) {
					// TODO: display error to user
					Log.e(LOG_TAG, e.toString());
				}
				//mHandler.sendEmptyMessage(0);
			}
		}).start();
	}	
	
	protected void setupSIP(Context context) {
		// Setup the SIP preferences
		SharedPreferences settings = context.getSharedPreferences(RegisterAccount.PREFS_FILE, Context.MODE_PRIVATE);
		
		SharedPreferences sipSettings = context.getSharedPreferences(RegisterAccount.SIPDROID_PREFS, Context.MODE_PRIVATE);
		Editor ed = sipSettings.edit();
		
		String num = settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null);
		String email = settings.getString(RegisterAccount.PREF_EMAIL, null);
		
		ed.putString(Settings.PREF_USERNAME, settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null));
		ed.putString(Settings.PREF_PASSWORD, settings.getString(RegisterAccount.PREF_SECRET, null));
		ed.putString(Settings.PREF_SERVER, DEFAULT_SIP_SERVER);
		
		Log.v(LOG_TAG, "Configured prefs for number " + num + ", email " + email);
		
		ed.commit();
	}
	
}
