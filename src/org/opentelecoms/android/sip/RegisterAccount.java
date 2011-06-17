/*
 * Copyright (C) 2010 The Sipdroid Open Source Project
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
package org.opentelecoms.android.sip;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.net.URL;
import java.security.Key;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;
import java.util.List;
import java.util.Random;
import java.util.regex.Pattern;

import javax.crypto.Cipher;

import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.opentelecoms.util.Base64;
import org.sipdroid.sipua.R;
import org.sipdroid.sipua.SipdroidEngine;
import org.sipdroid.sipua.ui.Settings;
import org.xmlpull.v1.XmlSerializer;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.text.format.Time;
import android.util.Log;
import android.util.Xml;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class RegisterAccount extends Activity {

	protected final static int PASSWORD_LEN = 12;
	
	public static final String PREFS_FILE = "org.opentelecoms.prefs.reg";
	public static final String PREF_PHONE_NUMBER = "phoneNumber";
	public static final String PREF_SECRET = "secret";
	public static final String PREF_FIRST_NAME = "firstName";
	public static final String PREF_LAST_NAME = "lastName";
	public static final String PREF_EMAIL = "emailAddr";
	
	// TODO: should get this from Settings.sharedPrefsFile somehow
	public static final String SIPDROID_PREFS = "org.sipdroid.sipua_preferences";
	
	private static final String TAG = "RegAcct";
	
	SharedPreferences settings;
	String password;
	
/* 	public static Boolean isPossible(Context context) {
		Boolean found = true; // disabled temporarily
	   	for (int i = 0; i < SipdroidEngine.LINES; i++) {
	   		String j = (i!=0?""+i:"");
	   		String username = PreferenceManager.getDefaultSharedPreferences(context).getString(Settings.PREF_USERNAME+j, Settings.DEFAULT_USERNAME),
	   			server = PreferenceManager.getDefaultSharedPreferences(context).getString(Settings.PREF_SERVER+j, Settings.DEFAULT_SERVER);
	   		if (username.equals("") || server.equals(""))
	   			continue;
	   		if (server.equals(Settings.DEFAULT_SERVER))
	   			found = true;
	   	}
	   	if (found) return false;
		Intent intent = new Intent(Intent.ACTION_SENDTO);
		intent.setPackage("com.google.android.apps.googlevoice");
		intent.setData(Uri.fromParts("smsto", "", null));
		List<ResolveInfo> a = context.getPackageManager().queryIntentActivities(intent,PackageManager.GET_INTENT_FILTERS);
		if (a == null || a.size() == 0)
			return false;
        Account[] accounts = AccountManager.get(context).getAccountsByType("com.google");
        for (Account account : accounts) {
        	  email = account.name;
        	  return true;
        }
        return false;
	} */
	
	String line;
	
    Handler mHandler = new Handler() {
    	public void handleMessage(Message msg) {
			Toast.makeText(RegisterAccount.this, line, Toast.LENGTH_LONG).show();
			buttonOK.setEnabled(true);
			//setCancelable(true);
    	}
    };

    String generatePassword(int length)	{
	    String availableCharacters = "";
	    String password = "";
	    
	    // Generate the appropriate character set
	    availableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	    availableCharacters = availableCharacters + "0123456789";
	    
	    // Generate the random number generator
	    Random selector = new Random();
	    
	    // Generate the password
	    int i;
	    for(i = 0; i < length; i++)
	    {
	            password = password + availableCharacters.charAt(selector.nextInt(availableCharacters.length() - 1));
	    }
	    
	    return password;
    }
    
    protected String getRegNum() {
    	return etNum.getText().toString();
    }
    
    protected String getPassword() {
    	if(password == null)
    		password = generatePassword(PASSWORD_LEN);
    	return password;
    }
	
	protected String getRegFirstName() {
		return etFirst.getText().toString();
	}
	
	protected String getRegLastName() {
		return etLast.getText().toString();
	}
	
	protected String getRegEmail() {
		return etEmail.getText().toString();
	}
    
    protected void storeSettings() {
    	settings = getSharedPreferences(PREFS_FILE, MODE_PRIVATE);
    	
    	Editor ed = settings.edit();
    	ed.putString(PREF_PHONE_NUMBER, getRegNum());
    	ed.putString(PREF_SECRET, getPassword());
    	ed.putString(PREF_FIRST_NAME, getRegFirstName());
    	ed.putString(PREF_LAST_NAME, getRegLastName());
    	ed.putString(PREF_EMAIL, getRegEmail());
    	ed.commit();
    }
	
	protected String getBodyXml() {

		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "registration");
		
			RegistrationUtil.serializeProperty(serializer, "phoneNumber", getRegNum());
			RegistrationUtil.serializeProperty(serializer, "secret", getPassword());
			RegistrationUtil.serializeProperty(serializer, "firstName", getRegFirstName());
			RegistrationUtil.serializeProperty(serializer, "lastName", getRegLastName());
			RegistrationUtil.serializeProperty(serializer, "emailAddress", getRegEmail());
			
			serializer.endTag("", "registration");
			serializer.endDocument();
			return writer.toString();
		} catch (Exception e) {
			throw new RuntimeException(e);
		} 
	}
	
	protected String getEncryptedXml() {
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			serializer.startTag("", "encryptedRegistration");
			serializer.attribute("", "regNum", getRegNum());

			String fullBody = getBodyXml();

			String encryptedBody = RegistrationUtil.getEncryptedStringAsBase64(this, fullBody); 
			serializer.text(encryptedBody);

			serializer.endTag("", "encryptedRegistration");
			serializer.endDocument();
			//return writer.toString();
			return encryptedBody;
		} catch (Exception e) {
			Log.e(TAG, e.toString());
			return null;
		}
	}
		
	void RegisterAccountNow() {
		buttonOK.setEnabled(false);
		//setCancelable(false);
		storeSettings();
		Toast.makeText(this, R.string.reg_please_wait, Toast.LENGTH_LONG).show();
        (new Thread() {
			public void run() {
				line = "Can't connect to webserver";
				try {
					
					// TODO: tidy up error handling, etc
					
					RegistrationUtil.submitMessage(getEncryptedXml());  

					/*
							Editor edit = PreferenceManager.getDefaultSharedPreferences(mContext).edit();
							edit.putString(Settings.PREF_SERVER, Settings.DEFAULT_SERVER);
							edit.putString(Settings.PREF_USERNAME, etName.getText()+"-200");
							edit.putString(Settings.PREF_DOMAIN, Settings.DEFAULT_DOMAIN);
							edit.putString(Settings.PREF_FROMUSER, Settings.DEFAULT_FROMUSER);
							edit.putString(Settings.PREF_PORT, Settings.DEFAULT_PORT);
							edit.putString(Settings.PREF_PROTOCOL, "tcp");
							edit.putString(Settings.PREF_PASSWORD, password);
							edit.commit();
				        	Receiver.engine(mContext).updateDNS();
				       		Receiver.engine(mContext).halt();
				   			Receiver.engine(mContext).StartEngine();
							dismiss();
						}
					} */
			        
				//} catch (IOException e) {
				//	e.printStackTrace();
				} catch (RegistrationFailedException e) {
					Log.e(TAG, e.toString());
				}
				mHandler.sendEmptyMessage(0);
			}
		}).start();   
	}

	EditText etNum, etFirst, etLast, etEmail;
	Button buttonOK;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.register_dialog);
        setTitle("Complete service activation");
        buttonOK = (Button) findViewById(R.id.Button01);
		buttonOK.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				RegisterAccountNow();
			}
		});
        etNum = (EditText) findViewById(R.id.EditText01);
        etNum.setText("+");
        etFirst = (EditText) findViewById(R.id.EditText02);
        etLast = (EditText) findViewById(R.id.EditText03);
        etEmail = (EditText) findViewById(R.id.EditText04);
        
        // Try to guess the values...
        // TODO: if we can guess anything, skip the form and register 
        // automatically? The user can change settings later if they
        // want to
        // TODO: should ignore exceptions in these methods, and just
        // fall back to the form
        
        etNum.setText(getMyPhoneNumber());
        setAccountDetails();
    }
	
	// Requires android.permission.READ_PHONE_STATE
	private String getMyPhoneNumber() {
	    TelephonyManager mTelephonyMgr;
	    mTelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE); 
	    String num = mTelephonyMgr.getLine1Number();
	     
	    if(num.length() == 0) 
	    	num = getCountryCode(mTelephonyMgr.getSimCountryIso());
	    
	    return "+" + num;
	}
	
	// TODO: all ISO country codes
	private String getCountryCode(String countrySymbol) {
		if(countrySymbol.toUpperCase().equals("AU"))
			return "61";
		else if(countrySymbol.toUpperCase().equals("CH"))
			return "41";
		else if(countrySymbol.toUpperCase().equals("CL"))
			return "56";
		else if(countrySymbol.toUpperCase().equals("IE"))
			return "353";
		else if(countrySymbol.toUpperCase().equals("UK"))
			return "44";
		else if(countrySymbol.toUpperCase().equals("US"))
			return "1";
		
		return "";
	}
	
	// Requires android.permission.GET_ACCOUNTS
	private void setAccountDetails() {
		String firstName = null;
		String lastName = null;
		String email = null;
		
		AccountManager am = AccountManager.get(this);
		// Get all accounts
		Account[] accounts = am.getAccountsByType(null);
		
		// TODO: extract email address using regexp
		//Pattern p = Pattern.compile("");
		
		for(Account acct : accounts) {
			if(!(acct.name.indexOf('@') < 0)) {
				email = acct.name;
				etEmail.setText(email);
				return;
			}
		}
		
		
		
	}


}
