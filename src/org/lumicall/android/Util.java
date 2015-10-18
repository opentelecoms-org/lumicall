package org.lumicall.android;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import android.content.Context;
import android.content.res.Resources;
import android.telephony.TelephonyManager;
import android.util.Log;

public class Util {
	
	final static String TAG = "Util";

	public static Map<String,String> loadCountriesByMCC(Context context) {
		Map<String,String> countriesByMCC = new HashMap<String,String>();
		Resources res = context.getResources();
		try {
			InputStream i = res.openRawResource(R.raw.mcc);
			BufferedReader in = new BufferedReader(new InputStreamReader(i));
			String line = in.readLine();
			while(line != null) {
				if(line.length() > 3) {
					String iso = line.substring(0, 1);
					String mcc = line.substring(3);
					countriesByMCC.put(mcc, iso.toLowerCase());
				}
				line = in.readLine();
			}
			return countriesByMCC;
		} catch(Exception ex) {
			return null;
		}
	}

	final static String hexChars = "0123456789abcdef";
	public static String byteToHexString(byte[] buffer, int offset, int length) {
		if(buffer == null)
			return "<null buffer>";
		// FIXME - performance
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < length; i++) {
			byte b = buffer[offset + i];
			sb.append(hexChars.charAt((b & 0xf0) >> 4));
			sb.append(hexChars.charAt(b&0xf));
		}
		return sb.toString();
	}

	public static String detectCountry(Context context) {
		
		try {
			TelephonyManager mTelephonyMgr;
			mTelephonyMgr = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
			
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
					String mcc = homeOperator.substring(0, 3);
					Map<String,String> countriesByMCC = loadCountriesByMCC(context);
					if(countriesByMCC != null) {
						String iso2 = countriesByMCC.get(mcc);
						if(iso2 != null && iso2.length() == 2) {
							return iso2;
						}
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get ro.cdma.home.operator.numeric: " + ex.getMessage());
				}
			}
			try {
				String simCountry = mTelephonyMgr.getSimCountryIso();
				if(simCountry != null && simCountry.length() == 2) {
					return simCountry;
				}
			} catch (Exception ex) {
				// we just ignore and try the next one...
				Log.d(TAG, "failed to get simCountry: " + ex.getMessage());
			}
			try {
				String networkCountry = mTelephonyMgr.getNetworkCountryIso();
				if(networkCountry != null && networkCountry.length() == 2) {
					return networkCountry;
				}
			} catch (Exception ex) {
				// we just ignore and try the next one...
				Log.d(TAG, "failed to get networkCountry: " + ex.getMessage());
			}
		} catch (Exception e) {
			Log.d(TAG, "failed to get data from TelephonyManager: " + e.getMessage());
		}
		
		String locale = Locale.getDefault().toString();
		
		if(locale != null && locale.length() > 1) {
			int i = locale.indexOf('_');
			if(i >= 0) {
				return locale.substring(i, i+2);
			}
			return locale.substring(0, 2);
		}
		
		return null;
	}


}
