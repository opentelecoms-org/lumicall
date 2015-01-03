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

package org.sipdroid.sipua.ui;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.UUID;

import org.sipdroid.codecs.Codecs;
import org.sipdroid.media.RtpStreamReceiver;
import org.lumicall.android.R;
import org.sipdroid.sipua.Constants;
import org.sipdroid.sipua.SipdroidEngine;
import org.zoolu.sip.provider.SipStack;

import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Build;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.text.InputType;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.Toast;

public class Settings extends PreferenceActivity implements OnSharedPreferenceChangeListener, OnClickListener {
	// Current settings handler
	private static SharedPreferences settings;
	// Context definition
	private Settings context = null;

	// Path where to store all profiles - !!!should be replaced by some system variable!!!
	private final static String profilePath = "/sdcard/Sipdroid/";
	// Shared preference file name - !!!should be replaced by some system variable!!!
	// IMPORTANT: Android appears to store the CheckBoxPreference values into a file
	// called <packageName>_preferences.xml (rather than any arbitrary file we
	// specify here)
	// Therefore, by making this exactly the same name as the package name, we ensure
	// that we are writing to the same _preferences file where the checkbox values are kept
	public static final String sharedPrefsFile = "org.lumicall.android_preferences";
	// List of profile files available on the SD card
	private String[] profileFiles = null;
	// Which profile file to delete
	private int profileToDelete;
	
	// If this is changed, it must also be changed in the manifest
	public final static String URI_SCHEME = "sipdroid";

	// IDs of the menu items
	private static final int MENU_IMPORT = 0;
	private static final int MENU_DELETE = 1;
	private static final int MENU_EXPORT = 2;

	// All possible values of the PREF_PREF preference (see bellow) 
	public static final String VAL_PREF_PSTN = "PSTN";
	public static final String VAL_PREF_SIP = "SIP";
	public static final String VAL_PREF_SIPONLY = "SIPONLY";
	public static final String VAL_PREF_ASK = "ASK";

	/*-
	 * ****************************************
	 * **** HOW TO USE SHARED PREFERENCES *****
	 * ****************************************
	 * 
	 * If you need to check the existence of the preference key
	 *   in this class:		contains(PREF_USERNAME)
	 *   in other classes:	PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).contains(Settings.PREF_USERNAME) 
	 * If you need to check the existence of the key or check the value of the preference
	 *   in this class:		getString(PREF_USERNAME, "").equals("")
	 *   in other classes:	PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(Settings.PREF_USERNAME, "").equals("")
	 * If you need to get the value of the preference
	 *   in this class:		getString(PREF_USERNAME, DEFAULT_USERNAME)
	 *   in other classes:	PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(Settings.PREF_USERNAME, Settings.DEFAULT_USERNAME)
	 */

	// Name of the keys in the Preferences XML file
//	public static final String PREF_USERNAME = "username";
//	public static final String PREF_PASSWORD = "password";
//	public static final String PREF_SERVER = "server";
//	public static final String PREF_DOMAIN = "domain";
//	public static final String PREF_FROMUSER = "fromuser";
//	public static final String PREF_PORT = "port";
//	public static final String PREF_PROTOCOL = "protocol";
	public static final String PREF_WLAN = "wlan";
	public static final String PREF_3G = "3g";
	public static final String PREF_EDGE = "edge";
	public static final String PREF_VPN = "vpn";
	public static final String PREF_PREF = "pref";
	public static final String PREF_DIALING_INTEGRATION = "dialing_integration";
	public static final String PREF_SIP = "sip_pref_sip_identity";
	public static final String PREF_TEL = "tel_pref_sip_identity";
	public static final String PREF_AUTO_ON = "auto_on";
	public static final String PREF_AUTO_ONDEMAND = "auto_on_demand";
	public static final String PREF_AUTO_HEADSET = "auto_headset";
//	public static final String PREF_MWI_ENABLED = "MWI_enabled";
//	public static final String PREF_REGISTRATION = "registration";
	public static final String PREF_NOTIFY = "notify";
	public static final String PREF_NODATA = "nodata";
//	public static final String PREF_SIPRINGTONE = "sipringtone";
//	public static final String PREF_SEARCH = "search";
//	public static final String PREF_EXCLUDEPAT = "excludepat";
	public static final String PREF_AUDIO_MODE = "audio_mode";
	public static final String PREF_EARGAIN = "eargain";
	public static final String PREF_MICGAIN = "micgain";
	public static final String PREF_HEARGAIN = "heargain";
	public static final String PREF_HMICGAIN = "hmicgain";
	public static final String PREF_OWNWIFI = "ownwifi";
//	public static final String PREF_STUN = "stun";
//	public static final String PREF_STUN_SERVER = "stun_server";
//	public static final String PREF_STUN_SERVER_PORT = "stun_server_port";
	
	// MMTel configurations (added by mandrajg)
//	public static final String PREF_MMTEL = "mmtel";
//	public static final String PREF_MMTEL_QVALUE = "mmtel_qvalue";
	
	// Call recording preferences.
	//public static final String PREF_CALLRECORD = "callrecord";
	
//	public static final String PREF_PAR = "par";
//	public static final String PREF_IMPROVE = "improve";
//	public static final String PREF_POSURL = "posurl";
//	public static final String PREF_POS = "pos";
//	public static final String PREF_CALLBACK = "callback";
//	public static final String PREF_CALLTHRU = "callthru";
//	public static final String PREF_CALLTHRU2 = "callthru2";
	public static final String PREF_CODECS = "codecs_new";
	public static final String PREF_DNS = "dns";
	public static final String PREF_VQUALITY = "vquality";
	public static final String PREF_MESSAGE = "vmessage";
	public static final String PREF_BLUETOOTH = "bluetooth";
	public static final String PREF_KEEPON = "keepon";
	public static final String PREF_SELECTWIFI = "selectwifi";
	public static final String PREF_ACCOUNT = "account";
	public static final String PREF_IPV4 = "preferipv4";
	public static final String PREF_LOG_URI_PREFIX = "call_log_uri_prefix";
	public static final String PREF_LOG_URI_HACK = "call_log_uri_hack";
	public static final String PREF_PTT_ENABLE = "ptt_enable";
	public static final String PREF_AUTH_FULL_URI = "auth_full_uri";
	
	public static final String PREF_SIP_INSTANCE_ID = "sip.instance.id";
	
	public static final String PREF_GANGLIA_ENABLE = "ganglia_enable";
	public static final String PREF_GANGLIA_UUID_ENABLE = "ganglia_uuid_enable";
	public static final String PREF_GANGLIA_HEARTBEAT = "ganglia_heartbeat";
	public static final String PREF_GANGLIA_LOCATION = "ganglia_location";
	public static final String PREF_GANGLIA_DEST = "ganglia_dest";
	public static final String PREF_GANGLIA_PORT = "ganglia_port";
	public static final String PREF_GANGLIA_TTL = "ganglia_ttl";
	public static final String PREF_GANGLIA_INTERVAL = "ganglia_interval";
	
	// Default values of the preferences
	public static final String	DEFAULT_USERNAME = "";
	public static final String	DEFAULT_PASSWORD = "";
	public static final String	DEFAULT_SERVER = "pbxes.org";
	public static final String	DEFAULT_DOMAIN = "";
	public static final String	DEFAULT_FROMUSER = "";
	public static final String	DEFAULT_PORT = "" + SipStack.default_port;
	public static final String	DEFAULT_PROTOCOL = "tcp";
	public static final boolean	DEFAULT_WLAN = true;
	public static final boolean	DEFAULT_3G = false;
	public static final boolean	DEFAULT_EDGE = false;
	public static final boolean	DEFAULT_VPN = false;
	public static final boolean	DEFAULT_DIALING_INTEGRATION = true;
	public static final String	DEFAULT_PREF = VAL_PREF_SIP;
	public static final boolean	DEFAULT_AUTO_ON = false;
	public static final boolean	DEFAULT_AUTO_ONDEMAND = false;
	public static final boolean	DEFAULT_AUTO_HEADSET = false;
	public static final boolean	DEFAULT_MWI_ENABLED = true;
	public static final boolean DEFAULT_REGISTRATION = true;
	public static final boolean	DEFAULT_NOTIFY = false;
	public static final boolean	DEFAULT_NODATA = false;
	public static final String	DEFAULT_SIPRINGTONE = "";
	public static final String	DEFAULT_SEARCH = "";
	public static final String	DEFAULT_EXCLUDEPAT = "";
	public static final float	DEFAULT_EARGAIN = (float) 0.25;
	public static final float	DEFAULT_MICGAIN = (float) 0.25;
	public static final float	DEFAULT_HEARGAIN = (float) 0.25;
	public static final float	DEFAULT_HMICGAIN = (float) 1.0;
	public static final boolean	DEFAULT_OWNWIFI = false;
	public static final boolean	DEFAULT_STUN = false;
	public static final String	DEFAULT_STUN_SERVER = "stun.ekiga.net";
	public static final String	DEFAULT_STUN_SERVER_PORT = "3478";
	
	// MMTel configuration (added by mandrajg)
	public static final boolean	DEFAULT_MMTEL = false;
	public static final String	DEFAULT_MMTEL_QVALUE = "1.00";	

	// Call recording preferences.
	public static final boolean DEFAULT_CALLRECORD = false;
	
	public static final boolean	DEFAULT_PAR = false;
	public static final boolean	DEFAULT_IMPROVE = false;
	public static final String	DEFAULT_POSURL = "";
	public static final boolean	DEFAULT_POS = false;
	public static final boolean	DEFAULT_CALLBACK = false;
	public static final boolean	DEFAULT_CALLTHRU = false;
	public static final String	DEFAULT_CALLTHRU2 = "";
	public static final String	DEFAULT_CODECS = null;
	public static final String	DEFAULT_DNS = "";
	public static final String  DEFAULT_VQUALITY = "low";
	public static final boolean DEFAULT_MESSAGE = false;
	public static final boolean DEFAULT_BLUETOOTH = false;
	public static final boolean DEFAULT_KEEPON = true;
	public static final boolean DEFAULT_SELECTWIFI = false;
	public static final int     DEFAULT_ACCOUNT = 0;
	public static final boolean DEFAULT_IPV4 = true;  // false: allow IPv6 and IPv4, true: IPv4 only
	public static final String DEFAULT_LOG_URI_PREFIX = Constants.URI_PREFIX;
	public static final boolean DEFAULT_LOG_URI_HACK = true;
	public static final boolean DEFAULT_PTT_ENABLE = false;
	public static final boolean DEFAULT_AUTH_FULL_URI = false;
	
	public static final boolean DEFAULT_GANGLIA_ENABLE = false;
	public static final boolean DEFAULT_GANGLIA_UUID_ENABLE = false;
	public static final boolean DEFAULT_GANGLIA_HEARTBEAT = true;
	public static final boolean DEFAULT_GANGLIA_LOCATION = false;
	public static final String DEFAULT_GANGLIA_DEST = "239.2.11.71";
	public static final int DEFAULT_GANGLIA_PORT = 8649;
	public static final int DEFAULT_GANGLIA_TTL = 15; // hops
	public static final int DEFAULT_GANGLIA_INTERVAL = 5;  // seconds

	// An other preference keys (not in the Preferences XML file)
	public static final String PREF_OLDVALID = "oldvalid";
	public static final String PREF_SETMODE = "setmode";
	public static final String PREF_OLDVIBRATE = "oldvibrate";
	public static final String PREF_OLDVIBRATE2 = "oldvibrate2";
	public static final String PREF_OLDPOLICY = "oldpolicy";
	public static final String PREF_OLDRING = "oldring";
	public static final String PREF_AUTO_DEMAND = "auto_demand";
	public static final String PREF_WIFI_DISABLED = "wifi_disabled";
	public static final String PREF_ON_VPN = "on_vpn";
	public static final String PREF_NODEFAULT = "nodefault";
	public static final String PREF_NOPORT = "noport";
	public static final String PREF_ON = "on";
	public static final String PREF_PREFIX = "prefix";
	public static final String PREF_COMPRESSION = "compression";
	//public static final String PREF_RINGMODEx = "ringmodeX";
	//public static final String PREF_VOLUMEx = "volumeX";

	// Default values of the other preferences
	public static final boolean	DEFAULT_OLDVALID = false;
	public static final boolean	DEFAULT_SETMODE = false;
	public static final int		DEFAULT_OLDVIBRATE = 0;
	public static final int		DEFAULT_OLDVIBRATE2 = 0;
	public static final int		DEFAULT_OLDPOLICY = 0;
	public static final int		DEFAULT_OLDRING = 0;
	public static final boolean	DEFAULT_AUTO_DEMAND = false;
	public static final boolean	DEFAULT_WIFI_DISABLED = false;
	public static final boolean DEFAULT_ON_VPN = false;
	public static final boolean	DEFAULT_NODEFAULT = false;
	public static final boolean DEFAULT_NOPORT = false;
	public static final boolean	DEFAULT_ON = false;
	public static final String	DEFAULT_PREFIX = "";
	public static final String	DEFAULT_COMPRESSION = null;
	//public static final String	DEFAULT_RINGTONEx = "";
	//public static final String	DEFAULT_VOLUMEx = "";
	
	public static InCallAudioPreference getInCallAudioPreference() {
		InCallAudioPreference audioPref = null;
		String _audioPref = PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(PREF_AUDIO_MODE, "");
		audioPref = InCallAudioPreference.parseString(_audioPref);
		if(audioPref == null) {
			audioPref = InCallAudioPreference.DETECT;
			// For some devices, e.g. Galaxy Tab, speakerphone is a better default
			if(Build.MODEL.equals("GT-P7500")) {
				audioPref = InCallAudioPreference.FORCE_SPEAKER;
			}
		}
		return audioPref;
	}

	public static float getEarGain() {
		try {
			return Float.valueOf(PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(Receiver.headset > 0 ? PREF_HEARGAIN : PREF_EARGAIN, "" + DEFAULT_EARGAIN));
		} catch (NumberFormatException i) {
			return DEFAULT_EARGAIN;
		}			
	}

	public static float getMicGain() {
		if (Receiver.headset > 0 || Receiver.bluetooth > 0) {
			try {
				return Float.valueOf(PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(PREF_HMICGAIN, "" + DEFAULT_HMICGAIN));
			} catch (NumberFormatException i) {
				return DEFAULT_HMICGAIN;
			}			
		}

		try {
			return Float.valueOf(PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(PREF_MICGAIN, "" + DEFAULT_MICGAIN));
		} catch (NumberFormatException i) {
			return DEFAULT_MICGAIN;
		}			
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

    	if (Receiver.mContext == null) Receiver.mContext = this;
		addPreferencesFromResource(R.xml.preferences);
		setDefaultValues();
		Codecs.check();
	}
	
	void reload() {
		setPreferenceScreen(null);
		addPreferencesFromResource(R.xml.preferences);		
	}

	private void setDefaultValues() {
		settings = getSharedPreferences(sharedPrefsFile, MODE_PRIVATE);

		if (Sipdroid.market) {
			CheckBoxPreference cb = (CheckBoxPreference) getPreferenceScreen().findPreference(PREF_3G);
			cb.setChecked(false);
			CheckBoxPreference cb2 = (CheckBoxPreference) getPreferenceScreen().findPreference(PREF_EDGE);
			cb2.setChecked(false);
			getPreferenceScreen().findPreference(PREF_3G).setEnabled(false);
			getPreferenceScreen().findPreference(PREF_EDGE).setEnabled(false);
		}

		settings.registerOnSharedPreferenceChangeListener(this);

		updateSummaries();		
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
	    menu.add(0, MENU_IMPORT, 0, getString(R.string.settings_profile_menu_import)).setIcon(android.R.drawable.ic_menu_upload);
	    menu.add(0, MENU_EXPORT, 0, getString(R.string.settings_profile_menu_export)).setIcon(android.R.drawable.ic_menu_save);
	    menu.add(0, MENU_DELETE, 0, getString(R.string.settings_profile_menu_delete)).setIcon(android.R.drawable.ic_menu_delete);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
    	context = this;

    	switch (item.getItemId()) {
            case MENU_IMPORT:
            	// Get the content of the directory
            	profileFiles = getProfileList();
            	if (profileFiles != null && profileFiles.length > 0) {
	            	// Show dialog with the files
	    			new AlertDialog.Builder(this)
	    			.setTitle(getString(R.string.settings_profile_dialog_profiles_title))
	    			.setIcon(android.R.drawable.ic_menu_upload)
	    			.setItems(profileFiles, profileOnClick)
	    			.show();
            	} else {
	                Toast.makeText(this, "No profile found.", Toast.LENGTH_SHORT).show();
            	}
                return true;
                
            case MENU_EXPORT:
            	//exportSettings();   // FIXME removed functionality
            	break;

            case MENU_DELETE:
            	// Get the content of the directory
            	profileFiles = getProfileList();
            	new AlertDialog.Builder(this)
                .setTitle(getString(R.string.settings_profile_dialog_delete_title))
                .setIcon(android.R.drawable.ic_menu_delete)
    			.setItems(profileFiles, new DialogInterface.OnClickListener() {
    				// Ask the user to be sure to delete it
    				public void onClick(DialogInterface dialog, int whichItem) {
        				profileToDelete = whichItem;
    					new AlertDialog.Builder(context)
    	                .setIcon(android.R.drawable.ic_dialog_alert)
    	                .setTitle(getString(R.string.settings_profile_dialog_delete_title))
    	                .setMessage(getString(R.string.settings_profile_dialog_delete_text, profileFiles[whichItem]))
    	                .setPositiveButton(android.R.string.ok, deleteOkButtonClick)
    	                .setNegativeButton(android.R.string.cancel, null)
    	                .show();
    				}
    			})
                .show();
                return true;
        }

        return false;
    }

    public static String[] getProfileList() {
    	File dir = new File(profilePath);
    	return dir.list();
    }
    
    public String getSharedPrefsFilename() {
    	StringBuilder sb = new StringBuilder();
    	sb.append("/data/data/");
    	sb.append(context.getPackageName());
    	sb.append("/shared_prefs/");
    	sb.append(sharedPrefsFile);
    	sb.append(".xml");
    	return sb.toString();
    }

	private OnClickListener profileOnClick = new DialogInterface.OnClickListener() {
		public void onClick(DialogInterface dialog, int whichItem) {
			int set = updateSleepPolicy();
			boolean message = settings.getBoolean(PREF_MESSAGE, DEFAULT_MESSAGE);

			try {
				copyFile(new File(profilePath + profileFiles[whichItem]), new File(getSharedPrefsFilename()));
            } catch (Exception e) {
                Toast.makeText(context, getString(R.string.settings_profile_import_error), Toast.LENGTH_SHORT).show();
                return;
            }

   			settings.unregisterOnSharedPreferenceChangeListener(context);
   			setDefaultValues();

           	// Restart the engine
       		Receiver.engine(context).halt();
   			Receiver.engine(context).StartEngine();
   			
   			reload();
   			settings.registerOnSharedPreferenceChangeListener(context);
   			updateSummaries();
   			if (set != updateSleepPolicy())
   				updateSleep();
   			if (message) {
   	    		Editor edit = settings.edit();
   	    		edit.putBoolean(PREF_MESSAGE, true);
   	    		edit.commit();
   			}
		}
	};

	private OnClickListener deleteOkButtonClick = new DialogInterface.OnClickListener() {
		public void onClick(DialogInterface dialog, int whichButton) {
        	File profile = new File(profilePath + profileFiles[profileToDelete]);
        	boolean rv = false;
        	// Check if the file exists and try to delete it
        	if (profile.exists()) {
        		rv = profile.delete();
        	}
        	if (rv) {
        		Toast.makeText(context, getString(R.string.settings_profile_delete_confirmation), Toast.LENGTH_SHORT).show();
        	} else {
        		Toast.makeText(context, getString(R.string.settings_profile_delete_error), Toast.LENGTH_SHORT).show();
        	}
		}
	};

    public void copyFile(File in, File out) throws Exception {
        FileInputStream  fis = new FileInputStream(in);
        FileOutputStream fos = new FileOutputStream(out);
        try {
            byte[] buf = new byte[1024];
            int i = 0;
            while ((i = fis.read(buf)) != -1) {
                fos.write(buf, 0, i);
            }
        } catch (Exception e) {
            throw e;
        } finally {
            if (fis != null) fis.close();
            if (fos != null) fos.close();
        }
    }

	@Override
	public void onDestroy()	{
		super.onDestroy();

		settings.unregisterOnSharedPreferenceChangeListener(this);
	}

	EditText transferText;
	String mKey;

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
    	if (!Thread.currentThread().getName().equals("main"))
    		return;
		if (key.startsWith(PREF_WLAN) ||
        			key.startsWith(PREF_3G) ||
        			key.startsWith(PREF_EDGE) ||
        			key.startsWith(PREF_VPN) ||
        			key.equals(PREF_AUTO_ONDEMAND) ||
        			key.equals(PREF_KEEPON)) {
        	Receiver.engine(this).halt();
    		Receiver.engine(this).StartEngine();
		}
		if (key.startsWith(PREF_WLAN) || key.startsWith(PREF_3G) || key.startsWith(PREF_EDGE) || key.startsWith(PREF_OWNWIFI)) {
			updateSleep();
		}

		updateSummaries();
    }

    int updateSleepPolicy() {
        ContentResolver cr = getContentResolver();
		int get = android.provider.Settings.System.getInt(cr, android.provider.Settings.System.WIFI_SLEEP_POLICY, -1);
		int set = get;
		boolean wlan = false,g3 = true,valid = false;
		for (int i = 0; i < Receiver.engine(Receiver.mContext).getLineCount(); i++) {
			String j = (i!=0?""+i:"");
		/*	if (!settings.getString(PREF_USERNAME+j, "").equals("") && 
					!settings.getString(PREF_SERVER+j, "").equals("")) {
				valid = true;
				wlan |= settings.getBoolean(PREF_WLAN+j, DEFAULT_WLAN);
				g3 &= settings.getBoolean(PREF_3G+j, DEFAULT_3G) ||
					settings.getBoolean(PREF_EDGE+j, DEFAULT_EDGE);
			} */
		}
		boolean ownwifi = settings.getBoolean(PREF_OWNWIFI, DEFAULT_OWNWIFI);

		if (g3 && valid && !ownwifi) {
			set = android.provider.Settings.System.WIFI_SLEEP_POLICY_DEFAULT;
		} else if (wlan || ownwifi) {
			set = android.provider.Settings.System.WIFI_SLEEP_POLICY_NEVER;
		}
		return set;
    }
    
	void updateSleep() {
        ContentResolver cr = getContentResolver();
		int get = android.provider.Settings.System.getInt(cr, android.provider.Settings.System.WIFI_SLEEP_POLICY, -1);
		int set = updateSleepPolicy();

		if (set != get) {
			Toast.makeText(this, set == android.provider.Settings.System.WIFI_SLEEP_POLICY_DEFAULT?
					R.string.settings_policy_default:R.string.settings_policy_never, Toast.LENGTH_LONG).show();
			android.provider.Settings.System.putInt(cr, android.provider.Settings.System.WIFI_SLEEP_POLICY, set);
		}
	}

	void fill(String pref,String def,int val,int disp) {
    	for (int i = 0; i < getResources().getStringArray(val).length; i++) {
        	if (settings.getString(pref, def).equals(getResources().getStringArray(val)[i])) {
        		getPreferenceScreen().findPreference(pref).setSummary(getResources().getStringArray(disp)[i]);
        	}
    	}
    }

	public void updateSummaries() {
    
       	//getPreferenceScreen().findPreference(PREF_SEARCH).setSummary(settings.getString(PREF_SEARCH, DEFAULT_SEARCH)); 
    	//getPreferenceScreen().findPreference(PREF_EXCLUDEPAT).setSummary(settings.getString(PREF_EXCLUDEPAT, DEFAULT_EXCLUDEPAT)); 
    	 
    	/* if (! settings.getString(PREF_PREF, DEFAULT_PREF).equals(VAL_PREF_PSTN)) {
    		getPreferenceScreen().findPreference(PREF_PAR).setEnabled(true);
    	} else {
    		getPreferenceScreen().findPreference(PREF_PAR).setEnabled(false);
      	} */
    	fill(PREF_EARGAIN,  "" + DEFAULT_EARGAIN,  R.array.eargain_values, R.array.eargain_display_values);
    	fill(PREF_MICGAIN,  "" + DEFAULT_MICGAIN,  R.array.eargain_values, R.array.eargain_display_values);
    	fill(PREF_HEARGAIN, "" + DEFAULT_HEARGAIN, R.array.eargain_values, R.array.eargain_display_values);
    	fill(PREF_HMICGAIN, "" + DEFAULT_HMICGAIN, R.array.eargain_values, R.array.eargain_display_values);
    	fill(PREF_VQUALITY,      DEFAULT_VQUALITY, R.array.vquality_values,R.array.vquality_display_values);
    	
    	getPreferenceScreen().findPreference(PREF_BLUETOOTH).setEnabled(RtpStreamReceiver.isBluetoothSupported());
    	
    	getPreferenceScreen().findPreference(PREF_GANGLIA_ENABLE).setDefaultValue(settings.getBoolean(Settings.PREF_GANGLIA_ENABLE, Settings.DEFAULT_GANGLIA_ENABLE));
    	getPreferenceScreen().findPreference(PREF_GANGLIA_HEARTBEAT).setDefaultValue(settings.getBoolean(Settings.PREF_GANGLIA_HEARTBEAT, Settings.DEFAULT_GANGLIA_HEARTBEAT));
    	getPreferenceScreen().findPreference(PREF_GANGLIA_DEST).setSummary(settings.getString(Settings.PREF_GANGLIA_DEST, Settings.DEFAULT_GANGLIA_DEST));
        getPreferenceScreen().findPreference(PREF_GANGLIA_PORT).setSummary("" + getStringAsInt(settings, Settings.PREF_GANGLIA_PORT, Settings.DEFAULT_GANGLIA_PORT));
        getPreferenceScreen().findPreference(PREF_GANGLIA_TTL).setSummary("" + getStringAsInt(settings, Settings.PREF_GANGLIA_TTL, Settings.DEFAULT_GANGLIA_TTL));
        getPreferenceScreen().findPreference(PREF_GANGLIA_INTERVAL).setSummary("" + getStringAsInt(settings, Settings.PREF_GANGLIA_INTERVAL, Settings.DEFAULT_GANGLIA_INTERVAL));
        
        getPreferenceScreen().findPreference(PREF_IPV4).setDefaultValue(settings.getBoolean(PREF_IPV4, Settings.DEFAULT_IPV4));
    }

	public static int getStringAsInt(SharedPreferences settings, String pref, int defaultValue) {
		return Integer.parseInt(settings.getString(pref, "" + defaultValue));
	}

    @Override
	public void onClick(DialogInterface arg0, int arg1) {
		Editor edit = settings.edit();
 		edit.putString(mKey, transferText.getText().toString());
		edit.commit();
	}

	public static String getSIPInstanceId() {
		return getSIPInstanceId(Receiver.mContext);
	}
		
	public static String getSIPInstanceId(Context context) {
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
		
		String instanceId = 
				sp.getString(PREF_SIP_INSTANCE_ID, "");
		if(instanceId.equals("")) {
			// Create and persist an instance ID for SIP
			instanceId = UUID.randomUUID().toString();
			Editor edit = sp.edit();
			edit.putString(PREF_SIP_INSTANCE_ID, instanceId);
			edit.commit();
		}
		
		return instanceId;
	}
}