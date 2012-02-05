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
package org.lumicall.android.sip;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.net.URL;
import java.security.Key;
import java.security.KeyFactory;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.util.Date;
import java.util.List;
import java.util.Locale;
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
import org.lumicall.android.R;
import org.lumicall.android.reg.EnrolmentService;
import org.sipdroid.sipua.SipdroidEngine;
import org.sipdroid.sipua.ui.Settings;
import org.xmlpull.v1.XmlSerializer;

import com.uecommerce.util.csv.CSVReader;

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
	
	public static final String PREFS_FILE = "org.opentelecoms.prefs.enrol";
	public static final String PREF_PHONE_NUMBER = "phoneNumber";
	public static final String PREF_SECRET = "secret";
	public static final String PREF_FIRST_NAME = "firstName";
	public static final String PREF_LAST_NAME = "lastName";
	public static final String PREF_EMAIL = "emailAddr";
	public static final String PREF_LAST_ENROLMENT_ATTEMPT = "lastEnrolmentAttempt";
	public static final String PREF_LAST_VALIDATION_ATTEMPT = "lastValidationAttempt";
	
	private static final String TAG = "EnrolAcct";
	
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
	
	int statusLine;
	
    Handler mHandler = new Handler() {
    	public void handleMessage(Message msg) {
			Toast.makeText(RegisterAccount.this, statusLine, Toast.LENGTH_LONG).show();
			buttonOK.setEnabled(true);
			//setCancelable(true);
    	}
    };

    String generatePassword(int length)	{
	    String availableCharacters = "";
	    StringBuilder password = new StringBuilder("");
	    
	    // Generate the appropriate character set
	    availableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	    availableCharacters = availableCharacters + "0123456789";
	    
	    // Generate the random number generator
	    SecureRandom selector = new SecureRandom();
	    
	    // Generate the password
	    int i;
	    for(i = 0; i < length; i++)
	    {
	            password.append(availableCharacters.charAt(selector.nextInt(availableCharacters.length())));
	    }
	    
	    return password.toString();
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
    	// Only change the times on a successful submission
    	//ed.putLong(PREF_LAST_REGISTRATION_ATTEMPT,
    	//		new Date().getTime() / 1000);
    	// Reset last activation attempt every time registration is
    	// attempted
    	ed.putLong(PREF_LAST_VALIDATION_ATTEMPT, 0);
    	ed.commit();
    }
    


	
	private void doValidationActivity() {
		final Intent intent = new Intent(RegisterAccount.this,
				ActivateAccount.class);
        Log.v(TAG, "enrolment sent, going to validation window");
        startActivity(intent);
        finish();
	}
	
	private void doMainActivity() {
		final Intent intent = new Intent(RegisterAccount.this,
				org.sipdroid.sipua.ui.Sipdroid.class);
		Log.v(TAG, "validation done already");
		startActivity(intent);
		finish();
	}
		
	void enrolAccountNow() {
		buttonOK.setEnabled(false);
		//setCancelable(false);
		storeSettings();
		
		final Intent intent = new Intent(this, EnrolmentService.class);
		intent.setAction(EnrolmentService.ACTION_ENROL);
		//intent.setAction(RegistrationPhaseTwo.ACTION);
		startService(intent);
		
		//doValidationActivity();  // FIXME - hiding the SMS validation temporarily
		finish();
		            
	}

	EditText etNum, etFirst, etLast, etEmail;
	Button buttonOK;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Has user already done registration or activation?
        SharedPreferences settings = getSharedPreferences(PREFS_FILE, MODE_PRIVATE);
        
        Long ts = settings.getLong(PREF_LAST_VALIDATION_ATTEMPT, 0);
        if(ts != 0) {
        	doMainActivity();
        	return;
        }
        
        ts = settings.getLong(PREF_LAST_ENROLMENT_ATTEMPT, 0);
        if(ts != 0) {
        	doValidationActivity();
        	return;
        }
        
        setContentView(R.layout.register_dialog);
        setTitle("Complete service activation");
        buttonOK = (Button) findViewById(R.id.Button01);
		buttonOK.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				enrolAccountNow();
			}
		});
        etNum = (EditText) findViewById(R.id.EditText01);
        etFirst = (EditText) findViewById(R.id.EditText02);
        etLast = (EditText) findViewById(R.id.EditText03);
        etEmail = (EditText) findViewById(R.id.EditText04);
        
        // Try to guess the values...
        // TODO: if we can guess anything, skip the form and register 
        // automatically? The user can change settings later if they
        // want to
        // TODO: should ignore exceptions in these methods, and just
        // fall back to the form
        
        String myPhoneNumber = getMyPhoneNumber();
        etNum.setText(settings.getString(PREF_PHONE_NUMBER, myPhoneNumber));
        setAccountDetails(settings);
    }
	
	// Requires android.permission.READ_PHONE_STATE
	private String getMyPhoneNumber() {
	    TelephonyManager mTelephonyMgr;
	    mTelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE); 
	    String num = mTelephonyMgr.getLine1Number();
	    
	    if(num == null || num.length() == 0) 
	    	num = getCountryCode(mTelephonyMgr.getSimCountryIso());
	    
	    return "+" + num;
	}
	
	// TODO: all ISO country codes
	private String getCountryCode(String countrySymbol) {
		
		try {
			CSVReader csv = new CSVReader(CountryData.class,
					new Class[] {String.class, String.class, String.class});
			
			Resources res = getResources();
			InputStream i = res.openRawResource(R.raw.country_codes);
			BufferedReader in = new BufferedReader(new InputStreamReader(i));
			CountryData cd = (CountryData)csv.read(in);
			while(cd != null) {
				if(countrySymbol.toUpperCase().equals(cd.getIsoCountryCode().toUpperCase()))
					return cd.getItuCountryCode();
				cd = (CountryData)csv.read(in);
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		// TODO: log not found		
		return "";
	}
	
	// Requires android.permission.GET_ACCOUNTS
	private void setAccountDetails(SharedPreferences settings) {
		String firstName = "";
		String lastName = "";
		String email = "";
		
		AccountManager am = AccountManager.get(this);
		// Get all accounts
		Account[] accounts = am.getAccountsByType(null);
		
		// TODO: extract email address using regexp
		//Pattern p = Pattern.compile("");
		
		for(Account acct : accounts) {
			if(email.equals("") && !(acct.name.indexOf('@') < 0)) {
				email = acct.name;
			}
		}
		
		etFirst.setText(settings.getString(PREF_FIRST_NAME, firstName));
        etLast.setText(settings.getString(PREF_LAST_NAME, lastName));
        etEmail.setText(settings.getString(PREF_EMAIL, email));
		
	}


}
