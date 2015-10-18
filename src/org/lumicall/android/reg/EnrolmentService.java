package org.lumicall.android.reg;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.lang.reflect.Method;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.lumicall.android.AppProperties;
import org.lumicall.android.R;
import org.lumicall.android.Util;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIP5060ProvisioningRequest;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.sip.ActivateAccount;
import org.lumicall.android.sip.RegisterAccount;
import org.lumicall.android.sip.RegistrationFailedException;
import org.lumicall.android.sip.RegistrationUtil;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;
import org.sipdroid.sipua.ui.Sipdroid;
import org.xmlpull.v1.XmlSerializer;

import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.Xml;

public class EnrolmentService extends IntentService {
	
	 private Notification notification;
     private NotificationManager nm;

	public static final String ACTION_ENROL_SMS = "org.lumicall.android.intent.USER_ENROL_SMS";
	public static final String ACTION_VALIDATE = "org.lumicall.android.intent.USER_VALIDATE";
	public static final String ACTION_SCAN_SMS_INBOX = "org.lumicall.android.intent.SCAN_SMS_INBOX";
	public static final String ACTION_RETRY_ENROLMENT = "org.lumicall.android.intent.RETRY_ENROLMENT";
	
	//public static final String ENROLMENT_MSG = "org.lumicall.android.extra.ENROLMENT_MSG";
	public static final String VALIDATION_CODE = "org.lumicall.android.extra.VALIDATION_CODE";
	
	public static final String VALIDATION_ATTEMPTED = "org.lumicall.android.extra.VALIDATION_ATTEMPTED";
	
	private static final String TAG = "EnrolSvc";
	private static final String PHASE2_PATTERN = "^(\\w+):(\\+\\d+)$";
	public static final String ACTION_SENT = "org.lumicall.SMS_SENT";
	public static final String ACTION_DELIVERED = "org.lumicall.SMS_DELIVERED";
	public static final String ACTION_SMS_FAILED = "org.lumicall.SMS_FAILED";
	public static final String SMS_REG_DEST = "Lumicall-Reg-Dest";
	public static final String SMS_REG_CODE = "Lumicall-Reg-Code";
	public static final String SMS_REG_RETRY_COUNT = "Lumicall-Reg-Retry-Count";
	
	public static final int SMS_INTERVAL = 20;
	public static final int DEFAULT_RETRY_COUNT = 3;
	
	Logger logger = Logger.getLogger(this.getClass().getCanonicalName());
	
	public EnrolmentService() {
		super("EnrolmentService");
	}
	
	/* BroadcastReceiver mReceiver = null;
	
	public class IncomingReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(EnrolmentService.VALIDATION_ATTEMPTED)) {
				Log.v(TAG, "notified that validation attempted, dismissing notifications");
				//EnrolmentService.this.nm.cancelAll();
			}
		}
	} */
	
	@Override
    public void onCreate() {
        super.onCreate();
        nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    }
	
	@Override
	protected void onHandleIntent(Intent intent) {
		if(intent.getAction().equals(ACTION_ENROL_SMS)) {
				
				
			doEnrolmentBySMS(this);
			
		} else if(intent.getAction().equals(ACTION_VALIDATE)) {
			// Get the validation code from the SMS
			String smsText = intent.getStringExtra(VALIDATION_CODE);
			handleValidationResponseSMS(smsText);
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
			
		} else if(intent.getAction().equals(ACTION_SMS_FAILED)) {
			// Failed to send the SMS
			
			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_sms_sending_failed), new Date().getTime());
			Intent notificationIntent = new Intent(this, Sipdroid.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_sms_sending_failed_title), getText(R.string.enrolment_sms_sending_failed), contentIntent);
	        nm.notify(10, notification);
	        LumicallDataSource ds = new LumicallDataSource(this);
            ds.open();
            for(SIP5060ProvisioningRequest req : ds.getSIP5060ProvisioningRequests())
				ds.deleteSIP5060ProvisioningRequest(req);
            ds.close();
            logger.warning("SMS sending failed, all provisioning requests deleted, user must start again");
			
		}
	}
	
	protected void validationDone(Context context, SIP5060ProvisioningRequest req) {
		Intent intent = new Intent();
		intent.setAction(EnrolmentService.VALIDATION_ATTEMPTED);
		this.sendBroadcast(intent);
		
		notification = new Notification(R.drawable.icon22, getText(R.string.validation_requested), new Date().getTime());
		Intent notificationIntent = new Intent(this, Sipdroid.class);
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
		notification.setLatestEventInfo(this, getText(R.string.validation_requested_title), getText(R.string.validation_requested_detail), contentIntent);
        nm.notify(10, notification);
		
		try {
			setupSIP(this, req);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	protected void makeValidationCall(Context context, String numberToDial) {
		String number = "tel:" + numberToDial;
        Intent callIntent = new Intent(Intent.ACTION_CALL, Uri.parse(number));
        callIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(callIntent);
	}
	
	
	protected void doEnrolmentBySMS(Context context) {
		try {
			AppProperties props;
			try {
				props = new AppProperties(context);
			} catch (IOException e) {
				throw new RegistrationFailedException(e.getClass().getCanonicalName() + ": " + e.getMessage());
			}
			
			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_request_detail), new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_request_title), getText(R.string.enrolment_request_detail), contentIntent);
            nm.notify(10, notification);
            
            LumicallDataSource ds = new LumicallDataSource(this);
            ds.open();
            List<SIP5060ProvisioningRequest> reqs = ds.getSIP5060ProvisioningRequests();
            if(reqs.size() < 1) {
            	logger.severe("no SIP5060ProvisioningRequest");
            	throw new RegistrationFailedException("no SIP5060ProvisioningRequest");
            }
            SIP5060ProvisioningRequest req = reqs.get(0);
			String enrolmentMessage = getEnrolmentBodyXml(req);
			String responseText = RegistrationUtil.submitMessage(props, "enrol", enrolmentMessage, req);
			ds.persistSIP5060ProvisioningRequest(req);
			
			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_requested_detail), new Date().getTime());
			notificationIntent = new Intent(this, ActivateAccount.class);
			contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_request_title), getText(R.string.enrolment_requested_detail), contentIntent);
            nm.notify(10, notification);
            
			logger.info("HTTP response: " + responseText);
			if(responseText.charAt(0) == '+') {
				String numberToDial = responseText;
				sendValidationSMS(context, numberToDial, req.getValidationCode1(), DEFAULT_RETRY_COUNT);
			} else if(responseText.equals("OK")) {
				logger.info("waiting for SMS");
			}
			ds.close();

		} catch (RegistrationFailedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			
			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_request_failed), new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_request_title), getText(R.string.enrolment_request_failed), contentIntent);
            nm.notify(10, notification);
		}
	}
	
	protected static PendingIntent getIntent(Context context, String action, String dest, String validationCode, int count) {
		Intent intent = new Intent(action);
		intent.putExtra(SMS_REG_DEST, dest);
		intent.putExtra(SMS_REG_CODE, validationCode);
		intent.putExtra(SMS_REG_RETRY_COUNT, count);
		PendingIntent pi = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
		return pi;
	}
	
	public static void sendValidationSMS(Context context, String dest, String validationCode, int count) {
		SmsManager smsManager = SmsManager.getDefault();
		smsManager.sendTextMessage(dest, null, validationCode,
				getIntent(context, ACTION_SENT, dest, validationCode, count),
				getIntent(context, ACTION_DELIVERED, dest, validationCode, count));
		Logger.getLogger(EnrolmentService.class.getCanonicalName())
			.info("Lumicall registration SMS sending initiated (sendTextMessage), retry count = " + count);
	}
	
	protected void handleValidationResponseSMS(String smsText) {

		try {
			Pattern p = Pattern.compile(PHASE2_PATTERN);
			Matcher m = p.matcher(smsText);
			if(!m.matches()) {
				// some other SMS, not for us
				logger.info("ignoring SMS (not for us), body = " + smsText);
				return;
			}
			String validationCode2 = m.group(1);
			String validatedNumber = m.group(2);
			
			LumicallDataSource ds = new LumicallDataSource(this);
			ds.open();
			List<SIP5060ProvisioningRequest> reqs = ds.getSIP5060ProvisioningRequests();
            if(reqs.size() < 1) {
            	logger.severe("no SIP5060ProvisioningRequest");
            	throw new RegistrationFailedException("no SIP5060ProvisioningRequest");
            }
            SIP5060ProvisioningRequest req = reqs.get(0);
				
			req.setPhoneNumber(validatedNumber);
			req.setValidationCode2(validationCode2);
			
			ds.persistSIP5060ProvisioningRequest(req);
			logger.info("validation reply SMS, code2 = " + validationCode2 + ", my number = " + validatedNumber);
			
			AppProperties props;
			try {
				props = new AppProperties(this);
			} catch (IOException e) {
				throw new RegistrationFailedException(e.getClass().getCanonicalName() + ": " + e.getMessage());
			}
			
			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_submitting_code), new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_request_title), getText(R.string.enrolment_submitting_code), contentIntent);
			nm.notify(10, notification);

			String s = getValidationBodyXml(validatedNumber, validationCode2);
			String responseText = RegistrationUtil.submitMessage(props, "validate", getValidationEncryptedXml(this, s), null);
			
			if(!responseText.startsWith("DONE")) {
				logger.severe("Bad response from validation server when sending phase2 code, text = " + responseText);
				return;
			}
			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_submitted_code), new Date().getTime());
			notificationIntent = new Intent(this, ActivateAccount.class);
			contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_request_title), getText(R.string.enrolment_submitted_code), contentIntent);
            nm.notify(10, notification);

			// This only gets updated if no exception occurred
			validationDone(this, req);
			
			ds.close();

		} catch (RegistrationFailedException e) {
			// TODO: display error to user
			Log.e(TAG, e.toString());

			notification = new Notification(R.drawable.icon22, getText(R.string.enrolment_submission_failed), new Date().getTime());
			Intent notificationIntent = new Intent(this, RegisterAccount.class);
			PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
			notification.setLatestEventInfo(this, getText(R.string.enrolment_request_title), getText(R.string.enrolment_submission_failed), contentIntent);
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

	protected String getHexEncodedKeySignatures() throws NameNotFoundException, NoSuchAlgorithmException, CertificateException {
		PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES);
		logger.fine("APK has " + packageInfo.signatures.length + " signature(s).");
		Set<String> signatures = new HashSet<String>();
		for (Signature signature : packageInfo.signatures) { // actually contains the public keys of the signers, not the signatures
			byte[] certificateBytes = signature.toByteArray();
			InputStream input = new ByteArrayInputStream(certificateBytes);
		    CertificateFactory cf = null;
		    cf = CertificateFactory.getInstance("X509");
		    X509Certificate c = null;
		    c = (X509Certificate) cf.generateCertificate(input);
		    MessageDigest md = MessageDigest.getInstance("SHA-1");
			md.update(c.getEncoded());
			byte[] digest = md.digest();
			signatures.add(Util.byteToHexString(digest, 0, digest.length));
		}
		StringBuffer result = new StringBuffer("");
		for(String signature : signatures) {
			if(result.length() > 0) {
				result.append(':');
			}
			result.append(signature);
		}
		logger.info("SHA-1 Signature(s) of keys used to sign the APK: " + result);
		return result.toString();
	}

	protected String getEnrolmentBodyXml(SIP5060ProvisioningRequest req) {
		
		SharedPreferences settings = getSharedPreferences(RegisterAccount.PREFS_FILE, Context.MODE_PRIVATE);
		
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			String ns = RegistrationUtil.NS;
			serializer.startTag(ns, "enrolment");
		
			String enrolmentNum = req.getPhoneNumber();
			if(enrolmentNum == null) {
				enrolmentNum = "";
			}
			
			RegistrationUtil.serializeProperty(serializer, ns, "phoneNumber", enrolmentNum);
			RegistrationUtil.serializeProperty(serializer, ns, "secret", req.getAuthPassword());
			RegistrationUtil.serializeProperty(serializer, ns, "firstName", "");
			RegistrationUtil.serializeProperty(serializer, ns, "lastName", "");
			RegistrationUtil.serializeProperty(serializer, ns, "emailAddress", "");
			RegistrationUtil.serializeProperty(serializer, ns, "language", getLanguage());
			try {
				RegistrationUtil.serializeProperty(serializer, ns, "signingKeys", getHexEncodedKeySignatures());
			} catch (Exception ex) {
				Log.w(TAG, "failed to get signing keys for myself", ex);
			}

			try {
				TelephonyManager mTelephonyMgr;
				mTelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);

				/**
				 * Privacy note:
				 * This information is gathered to help understand why the
				 * enrolment SMS request or reply does not get through some
				 * networks.
				 *
				 * The information below is not really private anyway, it is
				 * almost always possible for somebody in possession of the
				 * phone number to deduce which country the user is in and which
				 * network issued the number.
				 *
				 * Similar data is revealed by a user's IP address when browsing the
				 * Internet.
				 */

				//String deviceId = mTelephonyMgr.getDeviceId();

				String phoneType = null;
				try {
					int phoneTypeId = mTelephonyMgr.getPhoneType();
					switch(phoneTypeId) {
					case TelephonyManager.PHONE_TYPE_NONE:
						phoneType = "NONE";  // we can't actually do SMS enrolment in this case
						break;
					case TelephonyManager.PHONE_TYPE_GSM:
						phoneType = "GSM";
						break;
					case TelephonyManager.PHONE_TYPE_CDMA:
						phoneType = "CDMA";
						break;
					default:
						phoneType = "UNKNOWN:" + phoneTypeId;
					}
					RegistrationUtil.serializeProperty(serializer, ns, "phoneType", phoneType);
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get phoneType: " + ex.getMessage());
				}
				if (phoneType != null && phoneType.equals("CDMA")) {
					Class<?> c = null;
					Method get = null;
					try {
						// For CDMA phones
						c = Class.forName("android.os.SystemProperties");
						get = c.getMethod("get", String.class);
						// Gives MCC + MNC
						String homeOperator = ((String) get.invoke(c, "ro.cdma.home.operator.numeric"));
						RegistrationUtil.serializeProperty(serializer, ns, "cdmaHomeOperator", homeOperator);
					} catch (Exception ex) {
						// we just ignore and try the next one...
						Log.d(TAG, "failed to get ro.cdma.home.operator.numeric: " + ex.getMessage());
					}
					try {
						if (get != null) {
							String homeOperatorName = ((String) get.invoke(c, "ro.cdma.home.operator.alpha"));
							RegistrationUtil.serializeProperty(serializer, ns, "cdmaHomeOperatorName", homeOperatorName);
						}
					} catch (Exception ex) {
						// we just ignore and try the next one...
						Log.d(TAG, "failed to get ro.cdma.home.operator.alpha: " + ex.getMessage());
					}
				}
				try {
					String simCountry = mTelephonyMgr.getSimCountryIso();
					if(simCountry != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "simCountry", simCountry);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get simCountry: " + ex.getMessage());
				}
				try {
					String simOperator = mTelephonyMgr.getSimOperator();
					if(simOperator != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "simOperator", simOperator);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get simOperator: " + ex.getMessage());
				}
				try {
					String simOperatorName = mTelephonyMgr.getSimOperatorName();
					if(simOperatorName != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "simOperatorName", simOperatorName);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get simOperatorName: " + ex.getMessage());
				}
				try {
					String networkCountry = mTelephonyMgr.getNetworkCountryIso();
					if(networkCountry != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "networkCountry", networkCountry);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get networkCountry: " + ex.getMessage());
				}
				try {
					String networkOperator = mTelephonyMgr.getNetworkOperator();
					if(networkOperator != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "networkOperator", networkOperator);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get networkOperator: " + ex.getMessage());
				}
				try {
					String networkOperatorName = mTelephonyMgr.getNetworkOperatorName();
					if(networkOperatorName != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "networkOperatorName", networkOperatorName);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get networkOperatorName: " + ex.getMessage());
				}
				try {
					String line1Number = mTelephonyMgr.getLine1Number();
					if(line1Number != null) {
						RegistrationUtil.serializeProperty(serializer, ns, "line1Number", line1Number);
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get line1Number: " + ex.getMessage());
				}
			} catch (Exception ex) {
				// ignore, this part of the XML is optional
			}
			
			String packageName = getPackageName();
			RegistrationUtil.serializeProperty(serializer, ns, "packageName", packageName);
			try {
				PackageInfo packageInfo = getPackageManager().getPackageInfo(packageName, 0);
				RegistrationUtil.serializeProperty(serializer, ns, "versionName", packageInfo.versionName);
				RegistrationUtil.serializeProperty(serializer, ns, "versionCode", packageInfo.versionCode);
			} catch(NameNotFoundException ex) {
				Log.w(TAG, "failed to get PackageInfo for myself");
			}
			
			RegistrationUtil.serializeProperty(serializer, ns, "androidBuildVersionRelease", Build.VERSION.RELEASE);
			RegistrationUtil.serializeProperty(serializer, ns, "androidBuildVersionSDKINT", Build.VERSION.SDK_INT);
			RegistrationUtil.serializeProperty(serializer, ns, "androidBuildDisplay", Build.DISPLAY);
			
			serializer.endTag(ns, "enrolment");
			serializer.endDocument();
			return writer.toString();
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
	}
	
	protected String getEnrolmentEncryptedXml(SIP5060ProvisioningRequest req) {
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "encryptedEnrolment");
			//serializer.attribute("", "regNum", getRegNum());

			String fullBody = getEnrolmentBodyXml(req);

			String encryptedBody = RegistrationUtil.getEncryptedStringAsBase64(this, fullBody); 
			serializer.text(encryptedBody);

			serializer.endTag("", "encryptedEnrolment");
			serializer.endDocument();
			//return writer.toString();
			return encryptedBody;
		} catch (Exception e) {
			Log.e(TAG, e.toString());
			return null;
		}
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
			Log.e(TAG, e.toString());
			return null;
		}
	}	
	
	
	
	
	protected void setupSIP(Context context, SIP5060ProvisioningRequest req) throws IOException {
		
		AppProperties props = new AppProperties(context);

		// Setup the SIP preferences
		SharedPreferences settings = context.getSharedPreferences(RegisterAccount.PREFS_FILE, Context.MODE_PRIVATE);
		
		SharedPreferences sipSettings = context.getSharedPreferences(Settings.sharedPrefsFile, Context.MODE_PRIVATE);
		Editor edSIP = sipSettings.edit();
		
		String num = req.getPhoneNumber();
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		SIPIdentity sipIdentity = createSIPIdentity(props, settings, req);
		for(SIPIdentity s : ds.getSIPIdentities()) {
			if(s.getUri().equals(sipIdentity.getUri()))
				sipIdentity.setId(s.getId());
		}		
		ds.persistSIPIdentity(sipIdentity);
		ds.deleteSIP5060ProvisioningRequest(req);
		ds.close();
		edSIP.putString(Settings.PREF_SIP, Long.toString(sipIdentity.getId()));
		if(!sipSettings.contains(Settings.PREF_TEL))
			edSIP.putString(Settings.PREF_TEL, "-1");
		
		/* edSIP.putString(Settings.PREF_USERNAME, settings.getString(RegisterAccount.PREF_PHONE_NUMBER, null));
		edSIP.putString(Settings.PREF_PASSWORD, settings.getString(RegisterAccount.PREF_SECRET, null));
		edSIP.putString(Settings.PREF_SERVER, DEFAULT_SIP_SERVER);
		edSIP.putString(Settings.PREF_DOMAIN, DEFAULT_SIP_DOMAIN);
		edSIP.putString(Settings.PREF_PROTOCOL, "tcp");  // FIXME - change to TLS
		edSIP.putBoolean(Settings.PREF_STUN, true);
		edSIP.putString(Settings.PREF_STUN_SERVER, DEFAULT_STUN_SERVER);
		edSIP.putString(Settings.PREF_STUN_SERVER_PORT, "" + DEFAULT_STUN_SERVER_PORT); */
		edSIP.putBoolean(Settings.PREF_WLAN, true);
		edSIP.putBoolean(Settings.PREF_EDGE, true);
		edSIP.putBoolean(Settings.PREF_3G, true);
		edSIP.putBoolean(Settings.PREF_ON, true);
		
		if(edSIP.commit())
			Log.v(TAG, "Configured prefs for number " + num);
		else {
			Log.e(TAG, "error while committing preferences");
		}

		//Receiver.engine(context).updateDNS();
   		Receiver.engine(context).halt();
		Receiver.engine(context).StartEngine();
			
			
	}
	
	private SIPIdentity createSIPIdentity(AppProperties props, SharedPreferences settings, SIP5060ProvisioningRequest req) {
		
		SIPIdentity sipIdentity = new SIPIdentity();
		String uri = req.getPhoneNumber() +
				"@" + props.getSipDomain();
		sipIdentity.setUri(uri);
		sipIdentity.setAuthUser(uri);
		sipIdentity.setAuthPassword(req.getAuthPassword());
		sipIdentity.setReg(true);
		//sipIdentity.setRegServerName(props.getSipServer());
		sipIdentity.setRegServerName("");
		sipIdentity.setRegServerPort(props.getSipPort());
		sipIdentity.setRegServerProtocol(props.getSipProtocol());
		//sipIdentity.setOutboundServerName(props.getSipServer());
		sipIdentity.setOutboundServerName("");
		sipIdentity.setOutboundServerPort(props.getSipPort());
		sipIdentity.setOutboundServerProtocol(props.getSipProtocol());
		sipIdentity.setCarrierRoute(false);
		//sipIdentity.setStunServerName(props.getStunServer());
		sipIdentity.setStunServerName("");
		sipIdentity.setStunServerPort(props.getStunPort());
		sipIdentity.setStunServerProtocol("udp");
		return sipIdentity;
	}
	
}
