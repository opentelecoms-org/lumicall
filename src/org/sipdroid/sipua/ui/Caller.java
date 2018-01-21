package org.sipdroid.sipua.ui;

/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
 * 
 * This file is part of Sipdroid (http://www.sipdroid.org)
 * 
 * Sipdroid is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this source code; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.List;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

import org.sipdroid.media.RtpStreamReceiver;
import org.lumicall.android.R;
import org.lumicall.android.Util;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.sip.ENUMUtil;
import org.lumicall.android.sip.DialCandidateHelper;
import org.sipdroid.sipua.Constants;
import org.sipdroid.sipua.UserAgent;

import com.google.i18n.phonenumbers.NumberParseException;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.google.i18n.phonenumbers.Phonenumber.PhoneNumber;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.provider.Contacts;
import android.provider.Contacts.People;
import android.provider.Contacts.Phones;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import org.omnidial.harvest.DialCandidate;

public class Caller extends BroadcastReceiver {

		private static final int REDIAL_MINIMUM_INTERVAL = 3000;
		private static final String TAG = "Caller";
		static long noexclude;
		String last_number;
		long last_time;
		
		public class ChooserThread extends Thread {
			String number;
			String e164Number;
			Context context;
			public ChooserThread(Context context, String number, String e164Number) {
				this.context = context;
				this.number = number;
				this.e164Number = e164Number;
			}
			public void run() {
				try {
					Thread.sleep(200);
				} catch (InterruptedException e) {
				}
		        Intent intent = new Intent(Intent.ACTION_CALL,
		                Uri.fromParts(Settings.URI_SCHEME, number, null));
		        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		        intent.putExtra("number", number);
		        intent.putExtra("e164Number", e164Number);
		        context.startActivity(intent);					
			}
        }
		
		@Override
		public void onReceive(final Context context, Intent intent) {
	        String intentAction = intent.getAction();
	        String number = getResultData();
	        Boolean force = false;
	        
	        // This is quite an interesting area to log as a lot of
	        // decisions are made about call routing, so let's start with
	        // something to draw attention to it:
	        Log.d("SipUA:", "Caller.onReceive *****************************************************************************************************");
                Log.d(TAG, "number = " + number + " intent = " + intent + " data = " + intent.getData());
	        
	        if (intentAction.equals(Intent.ACTION_NEW_OUTGOING_CALL) && number != null)
	        {
        		if (!Sipdroid.release)
        			Log.i("SipUA:","outgoing call");
        		
        		// is sipdroid enabled?
        		if (!Sipdroid.on(context)) {
        			Log.d("SipUA:", "sipdroid not on");
        			return;
        		}
        		
			boolean bypassLumicall = false;
        		String originalUri = intent.getStringExtra("android.phone.extra.ORIGINAL_URI"); 
			if(originalUri != null) {
				Log.d(TAG, "originalUri = " + originalUri);
        			String uriFragment = Uri.parse(originalUri).getFragment();
				Log.d(TAG, "uriFragment = " + uriFragment);
        			if(uriFragment != null && uriFragment.contains(PSTN.BYPASS_LUMICALL)) {
					bypassLumicall = true;
				}
			}

			String gatewayPackage = intent.getStringExtra("com.android.phone.extra.GATEWAY_PROVIDER_PACKAGE");
			if(gatewayPackage != null)
			{
				bypassLumicall = true;
			}

			if(bypassLumicall)
			{
        			// Let the call go through to the next handler/GSM network
        			Log.i(TAG, "*** Lumicall detected `lumicall-bypass' in URI, letting call go to next handler ***");
        			setResultData(number);
        			abortBroadcast();
        			return;
	        	}
        		
        		boolean dialingIntegration =
        				PreferenceManager.getDefaultSharedPreferences(context).getBoolean(
        						Settings.PREF_DIALING_INTEGRATION, Settings.DEFAULT_DIALING_INTEGRATION);
        		boolean nonSIP = originalUri != null &&
					originalUri.length() > 4 &&
        				originalUri.startsWith("tel:") &&
        				(originalUri.charAt(4) == '+' ||
        				Character.isDigit(originalUri.charAt(4)));
        		if(!dialingIntegration && nonSIP) {
        			Log.d(TAG, "dialing integration not enabled");
        			return;
        		}
        		
        		// If the user has chosen a GSM route in the chooser, we should
        		// not do anything, just let the next handler deal with it
        		if(number.endsWith("?p")) {
        			setResultData(number.substring(0, number.indexOf('?')));
        			return;
        		}
        		
    			boolean sip_type = !PreferenceManager.getDefaultSharedPreferences(context).getString(Settings.PREF_PREF, Settings.DEFAULT_PREF).equals(Settings.VAL_PREF_PSTN);
    	        boolean ask = PreferenceManager.getDefaultSharedPreferences(context).getString(Settings.PREF_PREF, Settings.DEFAULT_PREF).equals(Settings.VAL_PREF_ASK);
    	        
      	        if (Receiver.call_state != UserAgent.UA_STATE_IDLE && RtpStreamReceiver.isBluetoothAvailable()) {
       	        	setResultData(null);
       	        	switch (Receiver.call_state) {
    	        	case UserAgent.UA_STATE_INCOMING_CALL:
    	        		Receiver.engine(context).answercall();
    	        		if (RtpStreamReceiver.bluetoothmode)
    	        			break;
    	        	default:
    	        		if (RtpStreamReceiver.bluetoothmode)
    	        			Receiver.engine(context).rejectcall();
    	        		else
    	        			Receiver.engine(context).togglebluetooth();
    	        		break;	
       	        	}
       	        	return;
      	        }
      	        
      	        // Don't redial without required interval between attempts
    	        if (last_number != null && last_number.equals(number) && (SystemClock.elapsedRealtime()-last_time) < REDIAL_MINIMUM_INTERVAL) {
    	        	setResultData(null);
    	        	abortBroadcast();
    	        	Log.w("SipUA:", "redial was too soon, aborted");
    	        	return;
    	        }
      	        last_time = SystemClock.elapsedRealtime();
    	        last_number = number;
    	        
    	        // Is the user over-riding the default network choice?
 				if (number.endsWith("+")) 
    			{
    				sip_type = !sip_type;
    				number = number.substring(0,number.length()-1);
    				force = true;
    			}
 				
				if (SystemClock.elapsedRealtime() < noexclude + 10000) {
					noexclude = 0;
					force = true;
				}
				
				String e164Number = null;
				
				boolean logUriHack = PreferenceManager.getDefaultSharedPreferences(context).getBoolean(Settings.PREF_LOG_URI_HACK, Settings.DEFAULT_LOG_URI_HACK);
				if (logUriHack) {
					int index = number.indexOf(Constants.SUBSTITUTE_AT);
					if (index >= 0) {
						Log.d(TAG, "found a substitute @, putting back regular @");
						String userPart = number.substring(0, index);
						String finalPart = number.substring(index+1);
						number = userPart + '@' + finalPart;
					} else if (originalUri != null) {
						try {
							String originalUriDecoded = URLDecoder.decode(originalUri, "UTF-8");
							Log.d(TAG, "decoded original URI: " + originalUriDecoded);
							for (String prefix : Constants.POSSIBLE_PREFIXES) {
								String prefixHack = "tel:" + prefix + ":";
								if (originalUriDecoded.startsWith(prefixHack)) {
									index = originalUriDecoded.indexOf(Constants.SUBSTITUTE_AT);
									if (index >= 0) {
										Log.d(TAG, "found encoded substitute @, putting back regular @");
										String userPart = originalUriDecoded.substring(prefixHack.length(), index);
										String finalPart = originalUriDecoded.substring(index+1);
										number = userPart + '@' + finalPart;
									} else {
										number = originalUriDecoded.substring(prefixHack.length());
									}
								}
							}
						} catch (UnsupportedEncodingException e1) {
							Log.e(TAG, "error decoding original URI: " + e1.getClass().getCanonicalName() + ": " + e1.getMessage());
						}
					}
				}
				if (number.indexOf('@') > 0) {
					// The target contains the @ symbol, treat it as a SIP address
					// NOTE: When somebody dials a SIP URI, it will usually not end up
					// in this receiver at all, the Intent generated by the platform will
					// launch the SIPUri Activity and the logic below is duplicated in that class.
					String _domain = number.substring(number.indexOf('@') + 1);
					LumicallDataSource ds = new LumicallDataSource(context);
					ds.open();
					
					List<SIPIdentity> sipIdentities = ds.getSIPIdentities();
					
					long sipIdentityId = -1;
					for(SIPIdentity s : sipIdentities) {
						String uri = s.getUri();
						String domain = uri.substring(uri.indexOf('@') + 1);
						if(domain.equals(_domain))
						{
							sipIdentityId = s.getId();
							Log.d(TAG, "matched domain: " + domain + ", using identity: " + s.getUri());
						}
					}
					
					ds.close();
					DialCandidate dc = new DialCandidate("sip", number, "", "Manual dial", sipIdentityId);
					if(!DialCandidateHelper.call(context, dc)) {
						// ignoring error
					}
					setResultData(null);
					abortBroadcast();
					return;
				}
				
				// FIXME - should not depend on the ENUM code like this
				boolean online = ENUMUtil.updateNotification(context);
				if(!online) {
					// Don't try to bother the user with the Lumicall dialing popup
					// if they are not connected to any Internet access.
					// Fall through to next event handler
					setResultData(number);
					return;
				}

				// Try and convert the target to an E.164 number
				// If it is already E.164 then this will remove punctuation too
				// FIXME - should prompt the user to check the number
				// FIXME - should update the contact DB
				String countryIsoCode = Util.detectCountry(context);
				if(countryIsoCode == null && number.charAt(0) == '+') {
					countryIsoCode = "CH";
				}
				if(countryIsoCode != null) {
					Log.v(TAG, "Converting number: " + number + ", country ISO = " + countryIsoCode);
					try {
						PhoneNumberUtil phoneUtil = PhoneNumberUtil.getInstance();
						PhoneNumber numberProto = phoneUtil.parse(number, countryIsoCode.toUpperCase());
						if(phoneUtil.isValidNumber(numberProto)) {
							e164Number = phoneUtil.format(numberProto, PhoneNumberFormat.E164);
						}
					} catch (NumberParseException e) {
						Log.w(TAG, "Error parsing number", e);
					}
				} else {
					Log.w(TAG, "Unable to resolve number to E.164 as country is unknown");
				}

				// Display a popup for the user to choose a candidate
				(new ChooserThread(context, number, e164Number)).start();  
				setResultData(null);
				abortBroadcast();
	        }
	    }

		private String concatenateNumbers(Context context, String _number, String callthru_number, String search) {
			String number = _number;
			/*	    					String orig = intent.getStringExtra("android.phone.extra.ORIGINAL_URI");	
				if (orig.lastIndexOf("/phones") >= 0) 
			{
					orig = orig.substring(0,orig.lastIndexOf("/phones")+7);
				Uri contactRef = Uri.parse(orig);
			 */
			Uri contactRef = Uri.withAppendedPath(Contacts.Phones.CONTENT_FILTER_URL, number);
			final String[] PHONES_PROJECTION = new String[] {
					People.Phones.NUMBER, // 0
					People.Phones.TYPE, // 1
			};
			Cursor phonesCursor = context.getContentResolver().query(contactRef, PHONES_PROJECTION, null, null,
					Phones.ISPRIMARY + " DESC");
			if (phonesCursor != null) {	        			        	
				number = "";
				while (phonesCursor.moveToNext()) {
					final int type = phonesCursor.getInt(1);
					String n = phonesCursor.getString(0);
					if (TextUtils.isEmpty(n))
						continue;
					if (type == Phones.TYPE_MOBILE || type == Phones.TYPE_HOME || type == Phones.TYPE_WORK) {
						if (!number.equals(""))
							number = number + "&";
						n = PhoneNumberUtils.stripSeparators(n);
						number = number + searchReplaceNumber(search, n);
					}
				}
				phonesCursor.close();
				if (number.equals(""))
					number = callthru_number;
			} else
				number = callthru_number;
			//			}			
			return number;
		}
		
		private String searchReplaceNumber(String pattern, String number) {
		    // Comma should be safe as separator.
		    String[] split = pattern.split(",");
		    // We need exactly 2 parts: search and replace. Otherwise
		    // we just return the current number.
		    if (split.length != 2)
			return number;

		    String modNumber = split[1];
		    
		    try {
			// Compiles the regular expression. This could be done
			// when the user modify the pattern... TODO Optimize
			// this, only compile once.
			Pattern p = Pattern.compile(split[0]);
    		    	Matcher m = p.matcher(number);
    		    	// Main loop of the function.
    		    	if (m.matches()) {
    		    	    for (int i = 0; i < m.groupCount() + 1; i++) {
    		    		String r = m.group(i);
    		    		if (r != null) {
    		    		    modNumber = modNumber.replace("\\" + i, r);
    		    		}
    		    	    }
    		    	}
    		    	// If the modified number is the same as the replacement
    		    	// value, we guess that the user typed a bad replacement
    		    	// value and we use the original number.
    		    	if (modNumber.equals(split[1])) {
    		    	    modNumber = number;
    		    	}
		    } catch (PatternSyntaxException e) {
			// Wrong pattern syntax. Give back the original number.
			modNumber = number;
		    }
		    
		    // Returns the modified number.
		    return modNumber;
		}
	    
	    Vector<String> getTokens(String sInput, String sDelimiter)
	    {
	    	Vector<String> vTokens = new Vector<String>();				
			int iStartIndex = 0;				
			final int iEndIndex = sInput.lastIndexOf(sDelimiter);
			for (; iStartIndex < iEndIndex; iStartIndex++) 
			{
				int iNextIndex = sInput.indexOf(sDelimiter, iStartIndex);
				String sPattern = sInput.substring(iStartIndex, iNextIndex).trim();
				vTokens.add(sPattern);
				iStartIndex = iNextIndex; 
			}
			if(iStartIndex < sInput.length())
				vTokens.add(sInput.substring(iStartIndex, sInput.length()).trim());
		
			return vTokens;
	    }
	    
	    boolean isExcludedNum(Vector<String> vExNums, String sNumber)
	    {
			for (int i = 0; i < vExNums.size(); i++) 
			{
				Pattern p = null;
				Matcher m = null;
				try
				{					
					p = Pattern.compile(vExNums.get(i));
					m = p.matcher(sNumber);	
				}
				catch(PatternSyntaxException pse)
				{
		           return false;    
				}  
				if(m != null && m.find())
					return true;			
			}    		
			return false;
	    }
	    
	    boolean isExcludedType(Vector<Integer> vExTypesCode, String sNumber, Context oContext)
	    {
	    	Uri contactRef = Uri.withAppendedPath(Contacts.Phones.CONTENT_FILTER_URL, sNumber);
	    	final String[] PHONES_PROJECTION = new String[] 
		    {
		        People.Phones.NUMBER, // 0
		        People.Phones.TYPE, // 1
		    };
	        Cursor phonesCursor = oContext.getContentResolver().query(contactRef, PHONES_PROJECTION, null, null,
	                null);
			if (phonesCursor != null) 
	        {	        			
 	            while (phonesCursor.moveToNext()) 
	            { 			            	
	                final int type = phonesCursor.getInt(1);	              
	                if(vExTypesCode.contains(Integer.valueOf(type)))
	                	return true;	    
	            }
	            phonesCursor.close();
	        }
			return false;
	    }   
	    
}
