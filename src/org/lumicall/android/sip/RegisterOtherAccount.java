/*
 * Copyright (C) 2015 The Lumicall Open Source Project
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

import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.lumicall.android.AppProperties;
import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIP5060ProvisioningRequest;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.ganglia.GMonitorService;
import org.lumicall.android.preferences.SIPIdentitiesSettings;
import org.lumicall.android.reg.EnrolmentService;
import org.sipdroid.sipua.SipdroidEngine;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;
import org.sipdroid.sipua.ui.Sipdroid;
import org.xmlpull.v1.XmlSerializer;

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

public class RegisterOtherAccount extends Activity {

	private static final String TAG = "OtherAcct";

	private static final int ABOUT_MENU_ITEM = 1;
	
	SharedPreferences settings;
	
	int statusLine;
	
	private EditText accountField;
	private EditText passwordField;

	private void doMainActivity() {
		final Intent intent = new Intent(RegisterOtherAccount.this,
				org.sipdroid.sipua.ui.Sipdroid.class);
		Log.v(TAG, "going to main activity");
		startActivity(intent);
		finish();
	}
	
	void advancedSetup() {
		/* settings = getSharedPreferences(RegisterAccount.PREFS_FILE, MODE_PRIVATE);
		Editor ed = settings.edit();
		long ts = new Date().getTime() / 1000;
		ed.putBoolean(RegisterAccount.PREF_ADVANCED_SETUP, true);
		ed.commit(); 

		doMainActivity(); */
		
		final Intent intent = new Intent(this, SIPIdentitiesSettings.class);
		intent.putExtra(SIPIdentity.SIP_IDENTITY_ID, Long.valueOf(-1));
        startActivity(intent);
        finish();
	}
	
	protected void doOtherAccountSetup() throws Exception {
		buttonDone.setEnabled(false);
		
		AppProperties props = new AppProperties(this);
		
        if(accountField == null) {
        	Log.e(TAG, "accountField == null");
        }
        if(passwordField == null) {
        	Log.e(TAG, "passwordField == null");
        }

		String sIPAccountId = accountField.getText().toString();
		String password = passwordField.getText().toString();

		SIPIdentity sipIdentity = new SIPIdentity();
		sipIdentity.setUri(sIPAccountId);
		sipIdentity.setAuthUser(sIPAccountId);   // do HA1b by default
		sipIdentity.setAuthPassword(password);
		sipIdentity.setReg(true);
		//sipIdentity.setRegServerName(props.getSipServer());
		sipIdentity.setRegServerName("");
		sipIdentity.setRegServerPort(props.getSipPort());
		sipIdentity.setRegServerProtocol(props.getSipProtocol());
		//sipIdentity.setOutboundServerName(props.getSipServer());
		sipIdentity.setOutboundServerName("");
		sipIdentity.setOutboundServerPort(props.getSipPort());
		sipIdentity.setOutboundServerProtocol(props.getSipProtocol());
		sipIdentity.setCarrierRoute(false); // FIXME - offer a checkbox for this?
		//sipIdentity.setStunServerName(props.getStunServer());
		sipIdentity.setStunServerName("");
		sipIdentity.setStunServerPort(props.getStunPort());
		sipIdentity.setStunServerProtocol("udp");
		
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();
		for(SIPIdentity s : ds.getSIPIdentities()) {
			if(s.getUri().equals(sipIdentity.getUri()))
				sipIdentity.setId(s.getId());
		}
		ds.persistSIPIdentity(sipIdentity);
		ds.close();
		Log.v(TAG, "Configured account: " + sIPAccountId);
		
		SharedPreferences sipSettings = getSharedPreferences(Settings.sharedPrefsFile, Context.MODE_PRIVATE);
		if(!sipSettings.contains(Settings.PREF_SIP)) {
			Editor edSIP = sipSettings.edit();
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
				Log.v(TAG, "Configured prefs");
			else {
				Log.e(TAG, "error while committing preferences");
			}
		}
		
		//Receiver.engine(this).updateDNS();
   		Receiver.engine(this).halt();
		Receiver.engine(this).StartEngine();
		
		doMainActivity();
	}
	
	protected void doManualVerification() {
		//storeSettings();
		final Intent intent = new Intent(this, ManualVerification.class);
		startActivity(intent);
		finish();
	}
			
	Button buttonDone;
	TextView advancedSettings;

	private Dialog m_AlertDlg;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.register_other_account_dialog);
        setTitle(R.string.reg_other_account_title);
        
        accountField = (EditText) findViewById(R.id.OtherAccountID);
        if(accountField == null) {
        	Log.e(TAG, "couldn't init accountField");
        }
        passwordField = (EditText) findViewById(R.id.OtherAccountPassword);
        if(passwordField == null) {
        	Log.e(TAG, "couldn't init passwordField");
        }
        

        buttonDone = (Button) findViewById(R.id.ButtonDone);
		buttonDone.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				try {
					doOtherAccountSetup();
				} catch (Exception ex) {
					Log.e(TAG, "couldn't create account", ex);
					buttonDone.setEnabled(true);
				}
			}
		});
		advancedSettings = (TextView) findViewById(R.id.AdvancedSetup);
		advancedSettings.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				advancedSetup();
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
	
}
