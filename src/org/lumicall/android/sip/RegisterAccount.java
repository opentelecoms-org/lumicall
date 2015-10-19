/*
 * Copyright (C) 2012-2015 The Lumicall Open Source Project
 * 
 * This file is part of Lumicall (http://lumicall.org)
 * 
 * Lumicall is free software; you can redistribute it and/or modify
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
import java.security.Security;
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
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIP5060ProvisioningRequest;
import org.lumicall.android.ganglia.GMonitorService;
import org.lumicall.android.reg.EnrolmentService;
import org.sipdroid.sipua.SipdroidEngine;
import org.sipdroid.sipua.ui.Settings;
import org.sipdroid.sipua.ui.Sipdroid;
import org.xmlpull.v1.XmlSerializer;

import com.google.i18n.phonenumbers.NumberParseException;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.google.i18n.phonenumbers.Phonenumber.PhoneNumber;
import org.opentelecoms.util.csv.CSVReader;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
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
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class RegisterAccount extends RegistrationActivity {

	public static final String PREFS_FILE = "org.opentelecoms.prefs.enrol";
	public static final String PREF_PHONE_NUMBER = "phoneNumber";
	public static final String PREF_ADVANCED_SETUP = "advancedSetupMode";
	
	private static final String TAG = "EnrolAcct";

	private static final int ABOUT_MENU_ITEM = 1;
	
	SharedPreferences settings;
	
	int statusLine;

	static {
		Security.insertProviderAt(new org.spongycastle.jce.provider.BouncyCastleProvider(), 1);
	}
	
    Handler mHandler = new Handler() {
    	public void handleMessage(Message msg) {
			Toast.makeText(RegisterAccount.this, statusLine, Toast.LENGTH_LONG).show();
			buttonOK.setEnabled(true);
			//setCancelable(true);
    	}
    };

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
		Log.v(TAG, "validation done or skipped, going to main activity");
		startActivity(intent);
		finish();
	}
	
	void otherProviderSetup() {
		final Intent intent = new Intent(RegisterAccount.this,
				RegisterOtherAccount.class);
		Log.v(TAG, "going to register other service account");
		startActivity(intent);
		finish();
	}
	
	protected void doSIP5060Provisioning() {
		buttonOK.setEnabled(false);
		storeSettings(null);
		
		final Intent intent = new Intent(this, EnrolmentService.class);
		intent.setAction(EnrolmentService.ACTION_ENROL_SMS);
		//intent.setAction(RegistrationPhaseTwo.ACTION);
		startService(intent);
		
		//doValidationActivity();  // FIXME - hiding the SMS validation temporarily
		finish();
	}
	
	protected void doManualVerification() {
		//storeSettings();
		final Intent intent = new Intent(this, ManualVerification.class);
		startActivity(intent);
		finish();
	}
			
	Button buttonOK;
	Button buttonManual;
	TextView otherProviderSetup;

	private Dialog m_AlertDlg;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        startService(new Intent(this, GMonitorService.class));
        
        // Has user already done registration or activation?
        LumicallDataSource ds = new LumicallDataSource(this);
        ds.open();
        int pendingAccounts = ds.getSIP5060ProvisioningRequests().size();
        int createdAccounts = ds.getSIPIdentities().size();
        ds.close();
        
        SharedPreferences settings = getSharedPreferences(PREFS_FILE, MODE_PRIVATE);
        
        boolean advancedSetup = settings.getBoolean(PREF_ADVANCED_SETUP, false);
        if(advancedSetup || createdAccounts > 0) {
        	doMainActivity();
        	return;
        }
        
        setContentView(R.layout.register_dialog);
        setTitle(R.string.reg_title);
        buttonOK = (Button) findViewById(R.id.Button01);
		buttonOK.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				doSIP5060Provisioning();
			}
		});
		buttonManual = (Button) findViewById(R.id.ButtonTryManual);
		buttonManual.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				doManualVerification();
			}
		});
		otherProviderSetup = (TextView) findViewById(R.id.OtherProviderSetup);
		otherProviderSetup.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				otherProviderSetup();
			}
		});
    }
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		boolean result = super.onCreateOptionsMenu(menu);

		MenuItem m = menu.add(0, ABOUT_MENU_ITEM, 0, R.string.menu_about);
		m.setIcon(android.R.drawable.ic_menu_info_details);
		
		return result;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		boolean result = super.onOptionsItemSelected(item);

		switch (item.getItemId()) {
		case ABOUT_MENU_ITEM:
			if (m_AlertDlg != null) 
			{
				m_AlertDlg.cancel();
			}
			m_AlertDlg = new AlertDialog.Builder(this)
				.setMessage(getString(R.string.about).replace("\\n","\n").replace("${VERSION}", Sipdroid.getVersion(this)))
				.setTitle(getString(R.string.menu_about))
				.setIcon(R.drawable.icon22)
				.setCancelable(true)
				.show();
			break;
		}
		return result;
	}
	
	// Requires android.permission.READ_PHONE_STATE
	private String getMyPhoneNumber() {
	    TelephonyManager mTelephonyMgr;
	    mTelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE); 
	    String num = mTelephonyMgr.getLine1Number();
	    
	    // MSISDN should not have a +, but some phones return it with a +
	    // We want to see a +
	    if(num != null && num.length() > 0 && !(num.charAt(0) == '+'))
	    	num = "+" + num;
	    
	    /* if(num == null || num.length() == 0) {
	    	PhoneHelper ph = new PhoneHelper();
	    	ph.init(this);
	    	num = ph.getLine1Number();
	    } */
	    
	    if(num == null || num.length() == 0) 
	    	num = "+" + getCountryCode(mTelephonyMgr.getSimCountryIso());
	    
	    return num;
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
	
}
