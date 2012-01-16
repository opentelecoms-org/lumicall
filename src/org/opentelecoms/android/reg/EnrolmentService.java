package org.opentelecoms.android.reg;

import java.io.StringWriter;
import java.util.Date;
import java.util.Locale;

import org.opentelecoms.android.sip.ActivateAccount;
import org.opentelecoms.android.sip.RegisterAccount;
import org.opentelecoms.android.sip.RegistrationFailedException;
import org.opentelecoms.android.sip.RegistrationUtil;
import org.sipdroid.sipua.R;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;
import org.xmlpull.v1.XmlSerializer;

import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.util.Log;
import android.util.Xml;

public class EnrolmentService extends IntentService {
	
	 private Notification notification;
     private NotificationManager nm;

	public static final String ACTION_ENROL = "org.opentelecoms.intent.USER_ENROL";
	public static final String ACTION_VALIDATE = "org.opentelecoms.intent.USER_VALIDATE";
	public static final String ACTION_SCAN_SMS_INBOX = "org.opentelecoms.intent.SCAN_SMS_INBOX";
	public static final String ACTION_RETRY_ENROLMENT = "org.opentelecoms.intent.RETRY_ENROLMENT";
	
	//public static final String ENROLMENT_MSG = "org.opentelecoms.extra.ENROLMENT_MSG";
	public static final String VALIDATION_CODE = "org.opentelecoms.extra.VALIDATION_CODE";
	
	private static final String DEFAULT_SIP_SERVER = "sip.lvdx.com";
	private static final String DEFAULT_SIP_DOMAIN = "lvdx.com";
	
	private static final String DEFAULT_STUN_SERVER = "stun.lvdx.com";
	private static final String DEFAULT_STUN_SERVER_PORT = "3478";
	
	private static final String LOG_TAG = "EnrolSvc";
	
	public EnrolmentService() {
		super("EnrolmentService");
	}
	
	@Override
    public void onCreate() {
        super.onCreate();
        nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    }

	
	@Override
	protected void onHandleIntent(Intent intent) {
		
		if(intent.getAction().equals(ACTION_ENROL)) {
			
			
			handleEnrolment(this);
			
		} else if(intent.getAction().equals(ACTION_VALIDATE)) {
			// Get the validation code from the SMS
			String regCode = intent.getStringExtra(VALIDATION_CODE);
			handleValidationCode(this, regCode);
			setupSIP(this);
		} else if(intent.getAction().equals(ACTION_SCAN_SMS_INBOX)) {
			
			// TODO
			// typically invoked when
			// - app starts
			// - phone boots
			
		} else if(intent.getAction().equals(ACTION_RETRY_ENROLMENT)) {
			
			// TODO - retry enrolment or validation
			// typically invoked
			// - phone boots
			// - network status changes
			// - app starts
			
		}
	}
	
	protected void handleEnrolment(Context context) {
		try {
			
			notification = new Notification(R.drawable.icon22, "Requesting enrolment...", new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, "Enrolment", "Requesting enrolment...", contentIntent);
            nm.notify(10, notification);
            
			String enrolmentMessage = getEnrolmentEncryptedXml();
			RegistrationUtil.submitMessage("enrol", enrolmentMessage);
			
			notification = new Notification(R.drawable.icon22, "Enrolment requested", new Date().getTime());
			notificationIntent = new Intent(this, ActivateAccount.class);
			contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, "Enrolment", "Enrolment requested", contentIntent);
            nm.notify(10, notification);
            
			// If we got here, it was successful
			updateSubmissionTimes();

		} catch (RegistrationFailedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			
			notification = new Notification(R.drawable.icon22, "Failed to submit enrolment", new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, "Enrolment", "Failed to submit enrolment", contentIntent);
            nm.notify(10, notification);
		}
	}

	protected void handleValidationCode(final Context context, final String regCode) {

		try {
			notification = new Notification(R.drawable.icon22, "Submitting validation code...", new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, "Enrolment", "Submitting validation code...", contentIntent);
			nm.notify(10, notification);

			SharedPreferences settings = context.getSharedPreferences(RegisterAccount.PREFS_FILE, Context.MODE_PRIVATE);
			String phoneNumber = settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null);
			String s = getValidationBodyXml(phoneNumber, regCode);
			RegistrationUtil.submitMessage("validate", getValidationEncryptedXml(context, s));  

			notification = new Notification(R.drawable.icon22, "Validation requested", new Date().getTime());
			notificationIntent = new Intent(this, ActivateAccount.class);
			contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, "Enrolment", "Validation requested", contentIntent);
            nm.notify(10, notification);

			// This only gets updated if no exception occurred
			Editor ed = settings.edit();
			ed.putLong(RegisterAccount.PREF_LAST_VALIDATION_ATTEMPT,
					new Date().getTime() / 1000);
			ed.commit();

		} catch (RegistrationFailedException e) {
			// TODO: display error to user
			Log.e(LOG_TAG, e.toString());

			notification = new Notification(R.drawable.icon22, "Failed to submit validation code", new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, "Enrolment", "Failed to submit validation code", contentIntent);
            nm.notify(10, notification);

		}

	}	
	
	/*
	 * @returns a two letter lowercase ISO language code
	 *          as defined by ISO 639-1, followed by
	 *          country code, then variant, separated
	 *          by underscores, as described in
	 *          java.util.Locale.toString() 
	 *          e.g. "en_US"
	 */
	protected String getLanguage() {
		return Locale.getDefault().toString();
	}

	protected String getEnrolmentBodyXml() {
		
		SharedPreferences settings = getSharedPreferences(RegisterAccount.PREFS_FILE, Context.MODE_PRIVATE);
		
    	String enrolmentNum = settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null);
    	String password = settings.getString(RegisterAccount.PREF_SECRET, null);
    	String firstName = settings.getString(RegisterAccount.PREF_FIRST_NAME, null);
    	String lastName = settings.getString(RegisterAccount.PREF_LAST_NAME, null);
    	String emailAddr = settings.getString(RegisterAccount.PREF_EMAIL, null);

		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			String ns = RegistrationUtil.NS;
			serializer.startTag(ns, "enrolment");
		
			RegistrationUtil.serializeProperty(serializer, ns, "phoneNumber", enrolmentNum);
			RegistrationUtil.serializeProperty(serializer, ns, "secret", password);
			RegistrationUtil.serializeProperty(serializer, ns, "firstName", firstName);
			RegistrationUtil.serializeProperty(serializer, ns, "lastName", lastName);
			RegistrationUtil.serializeProperty(serializer, ns, "emailAddress", emailAddr);
			RegistrationUtil.serializeProperty(serializer, ns, "language", getLanguage());
			
			serializer.endTag(ns, "enrolment");
			serializer.endDocument();
			return writer.toString();
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
	}
	
	protected String getEnrolmentEncryptedXml() {
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "encryptedEnrolment");
			//serializer.attribute("", "regNum", getRegNum());

			String fullBody = getEnrolmentBodyXml();

			String encryptedBody = RegistrationUtil.getEncryptedStringAsBase64(this, fullBody); 
			serializer.text(encryptedBody);

			serializer.endTag("", "encryptedEnrolment");
			serializer.endDocument();
			//return writer.toString();
			return encryptedBody;
		} catch (Exception e) {
			Log.e(LOG_TAG, e.toString());
			return null;
		}
	}
	
    protected void updateSubmissionTimes() {
    	SharedPreferences settings = getSharedPreferences(RegisterAccount.PREFS_FILE, MODE_PRIVATE);
    	
    	Editor ed = settings.edit();
    	ed.putLong(RegisterAccount.PREF_LAST_ENROLMENT_ATTEMPT,
    			new Date().getTime() / 1000);
    	// Reset last validation attempt every time enrolment is
    	// attempted
    	ed.putLong(RegisterAccount.PREF_LAST_VALIDATION_ATTEMPT, 0);
    	
    	ed.commit();
    }
	
	
	protected String getValidationBodyXml(String phoneNumber, String validationCode) {

		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			String ns = RegistrationUtil.NS;
			serializer.startTag(ns, "validation");
		
			RegistrationUtil.serializeProperty(serializer, ns, "phoneNumber", phoneNumber);
			RegistrationUtil.serializeProperty(serializer, ns, "validationCode", validationCode);
			
			serializer.endTag(ns, "validation");
			serializer.endDocument();
			return writer.toString();
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
	}
	
	protected String getValidationEncryptedXml(Context context, String s) {
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "encryptedValidation");
			//serializer.attribute("", "regNum", getRegNum());

			String encryptedText = RegistrationUtil.getEncryptedStringAsBase64(context, s);
			serializer.text(RegistrationUtil.getEncryptedStringAsBase64(context, s));

			serializer.endTag("", "encryptedValidation");
			serializer.endDocument();
			//return writer.toString();
			return encryptedText;
		} catch (Exception e) {
			Log.e(LOG_TAG, e.toString());
			return null;
		}
	}	
	
	
	
	
	protected void setupSIP(Context context) {
		// Setup the SIP preferences
		SharedPreferences settings = context.getSharedPreferences(RegisterAccount.PREFS_FILE, Context.MODE_PRIVATE);
		
		SharedPreferences sipSettings = context.getSharedPreferences(RegisterAccount.SIPDROID_PREFS, Context.MODE_PRIVATE);
		Editor edSIP = sipSettings.edit();
		
		String num = settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null);
		String email = settings.getString(RegisterAccount.PREF_EMAIL, null);
		
		edSIP.putString(Settings.PREF_USERNAME, settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null));
		edSIP.putString(Settings.PREF_PASSWORD, settings.getString(RegisterAccount.PREF_SECRET, null));
		edSIP.putString(Settings.PREF_SERVER, DEFAULT_SIP_SERVER);
		edSIP.putString(Settings.PREF_DOMAIN, DEFAULT_SIP_DOMAIN);
		edSIP.putString(Settings.PREF_PROTOCOL, "tcp");  // FIXME - change to TLS
		edSIP.putBoolean(Settings.PREF_STUN, true);
		edSIP.putString(Settings.PREF_STUN_SERVER, DEFAULT_STUN_SERVER);
		edSIP.putString(Settings.PREF_STUN_SERVER_PORT, DEFAULT_STUN_SERVER_PORT);
		edSIP.putBoolean(Settings.PREF_EDGE, true);
		edSIP.putBoolean(Settings.PREF_3G, true);
		edSIP.putBoolean(Settings.PREF_ON, true);
		
		Log.v(LOG_TAG, "Configured prefs for number " + num + ", email " + email);
		
		edSIP.commit();
		
		Receiver.engine(context).updateDNS();
   		Receiver.engine(context).halt();
		Receiver.engine(context).StartEngine();
			
			
	}
	
}
