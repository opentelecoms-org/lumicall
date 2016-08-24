/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
 * Copyright (C) 2008 Hughes Systique Corporation, USA (http://www.hsc.com)
 * Copyright (C) 2016 Pranav Jain
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

package org.sipdroid.sipua;

import java.io.IOException;
import java.io.InputStream;
import java.net.UnknownHostException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.cert.CertificateException;
import java.util.List;

import org.ice4j.StackProperties;
import org.lumicall.android.AndroidTimerFactory;
import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.sip.DialCandidateHelper;
import org.sipdroid.net.KeepAliveSip;
import org.sipdroid.sipua.ui.ChangeAccount;
import org.sipdroid.sipua.ui.LoopAlarm;
import org.sipdroid.sipua.ui.MessageSendingRequest;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;
import org.sipdroid.sipua.ui.Sipdroid;
import org.zoolu.net.IpAddress;
import org.zoolu.net.SocketAddress;
import org.zoolu.net.TcpSocket;
import org.zoolu.sip.address.NameAddress;
import org.zoolu.sip.provider.SipProvider;
import org.zoolu.sip.provider.SipStack;
import org.zoolu.tools.Timer;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.PowerManager;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.util.Log;

import org.omnidial.harvest.DialCandidate;

public class SipdroidEngine implements RegisterAgentListener {

	//public static final int LINES = 2;
	public int pref;
	
	public static final int UNINITIALIZED = 0x0;
	public static final int INITIALIZED = 0x2;
	
	/** User Agent */
	public UserAgent[] uas;
	public UserAgent ua;

	/** Register Agent */
	public RegisterAgent[] ras;

	/** Messaging */
	public MessageAgent[] mas;

	private KeepAliveSip[] kas;
	
	/** UserAgentProfile */
	public UserAgentProfile[] user_profiles;

	public SipProvider[] sip_providers;
	
	public static PowerManager.WakeLock[] wl,pwl;
	
	UserAgentProfile getUserAgentProfile(SIPIdentity sipIdentity) {
		UserAgentProfile user_profile = new UserAgentProfile(null);
		
		user_profile.sipIdentityId = sipIdentity.getId();
		
		user_profile.enable = sipIdentity.isEnable();
		
		//Uri uri = Uri.parse("sip:" + sipIdentity.getUri());
		String fromUri = sipIdentity.getUri();
		if (fromUri == null || fromUri.length() == 0) {
			Log.w(TAG, "Invalid From URI (null or empty)");
			return null;
		}
		int i = fromUri.indexOf('@');
		if (i < 1) {
			Log.w(TAG, "Invalid From URI, missing username or '@'");
			return null;
		}
		String fromUriDomain = fromUri.substring(i+1);
		String username = sipIdentity.getAuthUser();
		String password = sipIdentity.getAuthPassword();
		if (password == null || password.length() == 0) {
			// Do not try to authenticate if there is no password
			Log.d(TAG, "No password specified, will not try to authenticate");
			username = null;
			password = null;
		} else if (username == null || username.length() == 0) {
			// If authUser not specified, choose a value based on From URI
			
			if(PreferenceManager.getDefaultSharedPreferences(getUIContext()).getBoolean(Settings.PREF_AUTH_FULL_URI, Settings.DEFAULT_AUTH_FULL_URI)) {
				username = fromUri;
			} else {
				String fromUriUser = fromUri.substring(0, i);
				username = fromUriUser;
			}
			Log.i(TAG, "auth user was null/empty, using value derived from URI: " + username);
		}
		user_profile.username = username;
		user_profile.passwd = password;
		user_profile.realm = fromUriDomain;
		
		user_profile.realm_orig = user_profile.realm;
		user_profile.from_url = sipIdentity.getUri();
				
		user_profile.reg = sipIdentity.isReg();
		user_profile.reg_server_name = sipIdentity.getRegServerName();
		user_profile.reg_server_port = sipIdentity.getRegServerPort();
		user_profile.reg_server_protocol = sipIdentity.getRegServerProtocol();
		if(user_profile.reg && (user_profile.reg_server_name == null || user_profile.reg_server_name.length() == 0)) {
			user_profile.reg_server_name = user_profile.realm;
			user_profile.reg_server_port = 5061;
			user_profile.reg_server_protocol = "tls";
		}
		
		user_profile.qvalue = new Float(sipIdentity.getQ()).toString();
		user_profile.mmtel = sipIdentity.isMMTel();

		user_profile.pub = PreferenceManager.getDefaultSharedPreferences(getUIContext()).getBoolean(Settings.PREF_EDGE, Settings.DEFAULT_EDGE) ||
			PreferenceManager.getDefaultSharedPreferences(getUIContext()).getBoolean(Settings.PREF_3G, Settings.DEFAULT_3G);
		
		user_profile.mwi = sipIdentity.isMwi();
		
		user_profile.stun_server_name = sipIdentity.getStunServerName();
		if(user_profile.stun_server_name == null || user_profile.stun_server_name.length() == 0) {
			user_profile.stun_server_name = user_profile.realm;
		}
		if(!sipIdentity.isStun())
			user_profile.stun_server_name = null;
		user_profile.stun_server_port = sipIdentity.getStunServerPort();
		if(user_profile.stun_server_port < 0) 
			user_profile.stun_server_port = 3478;
		
		user_profile.ringTone = sipIdentity.getRingTone();
		
		user_profile.protocol = sipIdentity.getRegServerProtocol();
		
		user_profile.use_outbound = sipIdentity.isOutbound();
		user_profile.outbound_server_name = sipIdentity.getOutboundServerName();
		user_profile.outbound_server_port = sipIdentity.getOutboundServerPort();
		user_profile.outbound_server_protocol = sipIdentity.getOutboundServerProtocol();
		if(user_profile.use_outbound && (user_profile.outbound_server_name == null || user_profile.outbound_server_name.length() == 0)) {
			user_profile.outbound_server_name = user_profile.realm;
			user_profile.outbound_server_port = 5061;
			user_profile.outbound_server_protocol = "tls";
		}
		
		user_profile.security_mode = sipIdentity.getSecurityMode();
		
		user_profile.sipInstanceURN = "\"<urn:uuid:" + 
				org.sipdroid.sipua.ui.Settings.getSIPInstanceId() +
				">\"";
		
		return user_profile;
	}
	
	int lineCount = 0;

	private String TAG = "SipdroidEngine";
	public int getLineCount() {
		return lineCount;
	}

	public boolean StartEngine() {
		Context context = getUIContext();
		Timer.tf = new AndroidTimerFactory(context);
		
		KeyStore trusted;
		try {
			trusted = KeyStore.getInstance("BKS", "BC");
			InputStream in = context.getResources().openRawResource(R.raw.mytruststore);
			trusted.load(in, "".toCharArray());
			TcpSocket.setCustomKeyStore(trusted);
		} catch (Exception e) {
			Log.e(TAG , "Error setting up custom keystore");
			e.printStackTrace();
		}
		
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		List<SIPIdentity> sipIdentities = ds.getSIPIdentities();
		ds.close();
		lineCount = sipIdentities.size();
		
			PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
			if (wl == null  || lineCount != wl.length) {
				if(wl != null) {
					for(int i = 0; i < wl.length; i++) {
						if(wl[i] != null && wl[i].isHeld())
							wl[i].release();
						wl[i] = null;
						if(pwl[i] != null && pwl[i].isHeld())
							pwl[i].release();
						pwl[i] = null;
					}
				}
				if (!PreferenceManager.getDefaultSharedPreferences(getUIContext()).contains(org.sipdroid.sipua.ui.Settings.PREF_KEEPON)) {
					Editor edit = PreferenceManager.getDefaultSharedPreferences(getUIContext()).edit();
	
					edit.putBoolean(org.sipdroid.sipua.ui.Settings.PREF_KEEPON, Build.MODEL.equals("Nexus One") ||
							Build.MODEL.equals("Nexus S") ||
							Build.MODEL.equals("Archos5") ||
							Build.MODEL.equals("ADR6300") ||
							Build.MODEL.equals("PC36100") ||
							Build.MODEL.equals("HTC Desire") ||
							Build.MODEL.equals("HTC Incredible S") ||
							Build.MODEL.equals("HTC Wildfire") ||
							Build.MODEL.equals("GT-I9100"));
					edit.commit();
				}
				wl = new PowerManager.WakeLock[lineCount];
				pwl = new PowerManager.WakeLock[lineCount];
			}
			pref = ChangeAccount.getPref(Receiver.mContext);
			
			MessageManager messageManager = new MessageManager();

			uas = new UserAgent[lineCount];
			ras = new RegisterAgent[lineCount];
			mas = new MessageAgent[lineCount];
			kas = new KeepAliveSip[lineCount];
			lastmsgs = new String[lineCount];
			sip_providers = new SipProvider[lineCount];
			user_profiles = new UserAgentProfile[lineCount];
			for(int i = 0; i < lineCount; i++) {
				SIPIdentity sipIdentity = sipIdentities.get(i);
				user_profiles[i] = getUserAgentProfile(sipIdentity);
			}
			
			SipStack.init(null);
			int i = 0;
			for (UserAgentProfile user_profile : user_profiles) {
				if (wl[i] == null) {
					wl[i] = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Sipdroid.SipdroidEngine");
					if (PreferenceManager.getDefaultSharedPreferences(getUIContext()).getBoolean(org.sipdroid.sipua.ui.Settings.PREF_KEEPON, org.sipdroid.sipua.ui.Settings.DEFAULT_KEEPON))
						pwl[i] = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, "Sipdroid.SipdroidEngine");
				}
				
				try {
					SipStack.debug_level = 0;
		//			SipStack.log_path = "/data/data/org.sipdroid.sipua";
					SipStack.max_retransmission_timeout = 4000;
					SipStack.default_transport_protocols = new String[1];
					SipStack.default_transport_protocols[0] = user_profile.protocol;
					
					String version = context.getString(R.string.app_name) +
							"/" + Sipdroid.getVersion() + "/" + Build.MODEL;
					SipStack.ua_info = version;
					SipStack.server_info = version;
					// for ice4j:
					System.setProperty(StackProperties.SOFTWARE, context.getString(R.string.app_name));
						
					IpAddress.setLocalIpAddress();
					sip_providers[i] = new IntegratedSipProvider(IpAddress.localIpAddress, 0);
					user_profile.contact_url = getContactURL(user_profile.username,sip_providers[i]);
					
					if (user_profile.from_url.indexOf("@") < 0) {
						user_profile.from_url +=
							"@"
							+ user_profile.realm;
					}
					
					CheckEngine();
					
					// added by mandrajg
					String icsi = null;
					if (user_profile.mmtel == true){
						icsi = "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"";
					}
		
					uas[i] = ua = new UserAgent(sip_providers[i], user_profile);
					if(user_profile.enable && user_profile.reg){ 
						SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
						boolean publish_enable_status = prefs.getBoolean("publish_enable", true);
						ras[i] = new RegisterAgent(sip_providers[i], user_profile.from_url, // modified
							user_profile.contact_url, user_profile.username,
							user_profile.realm, user_profile.passwd, this, user_profile,
							user_profile.qvalue, icsi, user_profile.pub, user_profile.mwi, publish_enable_status);
						mas[i] = new MessageAgent(sip_providers[i], user_profile, messageManager);
						mas[i].receive();
					} else {
						ras[i] = null;
						mas[i] = null;
					}
					kas[i] = new KeepAliveSip(sip_providers[i],100000);
				} catch (Exception E) {
					Log.e(getClass().getCanonicalName(), "Exception while setting up profile: " + 
							E.getClass().getCanonicalName() + ": " +
							E.getMessage());
				}
				i++;
			}
			register();
			listen();

			return true;
	}
	
	private String getContactAddress() {
		String addr = IpAddress.localIpAddress;
		if(addr.indexOf(':') >= 0) {
			int zoneIndex = addr.indexOf('%');
			if(zoneIndex < 0)
				zoneIndex = addr.length();
			addr = '[' + addr.substring(0, zoneIndex) + ']';
		}
		
		return addr;
	}

	private String getContactURL(String username,SipProvider sip_provider) {
		int i = username.indexOf("@");
		if (i != -1) {
			// if the username already contains a @ 
			//strip it and everthing following it
			username = username.substring(0, i);
		}
		
		if(username.length() == 0) {
			username = "unspecified";
		}

		return username + "@" + getContactAddress()
		+ (sip_provider.getPort() != 0?":"+sip_provider.getPort():"")
		+ ";transport=" + sip_provider.getDefaultTransport();		
	}
	
	void setOutboundProxy(SipProvider sip_provider,int i) {
		try {
			if (sip_provider != null) sip_provider.setOutboundProxy(new SocketAddress(
					user_profiles[i].outbound_server_name,
					user_profiles[i].outbound_server_port
					));
		} catch (Exception e) {
		}
	}
	
	public void CheckEngine() {
		int i = 0;
		for (SipProvider sip_provider : sip_providers) {
			if (sip_provider != null && !sip_provider.hasOutboundProxy())
				setOutboundProxy(sip_provider,i);
			i++;
		}
	}

	public Context getUIContext() {
		return Receiver.mContext;
	}
	
	public int getRemoteVideo() {
		return ua.remote_video_port;
	}
	
	public int getLocalVideo() {
		return ua.local_video_port;
	}
	
	public String getRemoteAddr() {
		return ua.remote_media_address;
	}
	
	public void expire() {
		Receiver.expire_time = 0;
		int i = 0;
		for (RegisterAgent ra : ras) {
			if (ra != null && ra.CurrentState == RegisterAgent.REGISTERED) {
				ra.CurrentState = RegisterAgent.UNREGISTERED;
				Receiver.onText(Receiver.REGISTER_NOTIFICATION+i, null, 0, 0);
			}
			i++;
		}
		register();
	}
	
	public void unregister(int i) {
			if (user_profiles[i] == null ||
					user_profiles[i].username == null ||
					user_profiles[i].username.equals("") ||
					user_profiles[i].realm == null ||
					user_profiles[i].realm.equals("")) return;

			RegisterAgent ra = ras[i];
			if (ra != null && ra.unregister()) {
				Receiver.alarm(0, LoopAlarm.class);
				Receiver.onText(Receiver.REGISTER_NOTIFICATION+i,getUIContext().getString(R.string.reg),R.drawable.sym_presence_idle,0);
				wl[i].acquire();
			} else
				Receiver.onText(Receiver.REGISTER_NOTIFICATION+i, null, 0, 0);
	}

	public void registerMore() {
		IpAddress.setLocalIpAddress();
		int i = 0;
		for (RegisterAgent ra : ras) {
			try {
				if (user_profiles[i] == null || user_profiles[i].username.equals("") ||
						user_profiles[i].realm.equals("")) {
					i++;
					continue;
				}
				user_profiles[i].contact_url = getContactURL(user_profiles[i].from_url,sip_providers[i]);
		
				if (ra != null && !ra.isRegistered() && Receiver.isFast(i) && ra.register()) {
					Receiver.onText(Receiver.REGISTER_NOTIFICATION+i,getUIContext().getString(R.string.reg),R.drawable.sym_presence_idle,0);
					wl[i].acquire();
				}
			} catch (Exception ex) {
				
			}
			i++;
		}
	}
	
	public void register() {
		IpAddress.setLocalIpAddress();
		int i = 0;
		for (RegisterAgent ra : ras) {
			try {
				if (user_profiles[i] == null || user_profiles[i].username.equals("") ||
						user_profiles[i].realm.equals("")) {
					i++;
					continue;
				}
				user_profiles[i].contact_url = getContactURL(user_profiles[i].from_url,sip_providers[i]);
		
				if (!Receiver.isFast(i)) {
					unregister(i);
				} else {
					if (ra != null && ra.register()) {
						Receiver.onText(Receiver.REGISTER_NOTIFICATION+i,getUIContext().getString(R.string.reg),R.drawable.sym_presence_idle,0);
						wl[i].acquire();
					}
				}
			} catch (Exception ex) {
				
			}
			i++;
		}
	}
	
	public void registerUdp() {
		IpAddress.setLocalIpAddress();
		int i = 0;
		for (RegisterAgent ra : ras) {
			try {
				if (user_profiles[i] == null || user_profiles[i].username.equals("") ||
						user_profiles[i].realm.equals("") ||
						sip_providers[i] == null ||
						sip_providers[i].getDefaultTransport() == null ||
						sip_providers[i].getDefaultTransport().equals("tcp")) {
					i++;
					continue;
				}
				user_profiles[i].contact_url = getContactURL(user_profiles[i].from_url,sip_providers[i]);
		
				if (!Receiver.isFast(i)) {
					unregister(i);
				} else {
					if (ra != null && ra.register()) {
						Receiver.onText(Receiver.REGISTER_NOTIFICATION+i,getUIContext().getString(R.string.reg),R.drawable.sym_presence_idle,0);
						wl[i].acquire();
					}
				}
			} catch (Exception ex) {
				
			}
			i++;
		}
	}

	public void halt() { // modified
		long time = SystemClock.elapsedRealtime();
		
		int i = 0;
		for (RegisterAgent ra : ras) {
			if(ra != null)
				unregister(i);
			while (ra != null && ra.CurrentState != RegisterAgent.UNREGISTERED && SystemClock.elapsedRealtime()-time < 2000)
				try {
					Thread.sleep(100);
				} catch (InterruptedException e1) {
				}
			if (wl[i].isHeld()) {
				wl[i].release();
				if (pwl[i] != null && pwl[i].isHeld()) pwl[i].release();
			}
			if (kas[i] != null) {
				Receiver.alarm(0, LoopAlarm.class);
				kas[i].halt();
			}
			Receiver.onText(Receiver.REGISTER_NOTIFICATION+i, null, 0, 0);
			if (ra != null)
				ra.halt();
			if (uas[i] != null)
				uas[i].hangup();
			if (sip_providers[i] != null)
				sip_providers[i].halt();
			i++;
		}
	}

	public boolean isRegistered()
	{
		for (RegisterAgent ra : ras)
			if (ra != null && ra.isRegistered())
				return true;
		return false;
	}
	
	public boolean isRegistered(int i)
	{
		if (ras[i] == null)
		{
			return false;
		}
		return ras[i].isRegistered();
	}
	
	public void onUaRegistrationSuccess(RegisterAgent reg_ra, NameAddress target,
			NameAddress contact, String result) {
    	int i = 0;
    	for (RegisterAgent ra : ras) {
    		if (ra == reg_ra) break;
    		i++;
    	}
		if (isRegistered(i)) {
			if (Receiver.on_wlan)
				Receiver.alarm(60, LoopAlarm.class);
			Receiver.onText(Receiver.REGISTER_NOTIFICATION+i,getUIContext().getString(i == pref?R.string.regpref:R.string.regclick),R.drawable.sym_presence_available,0);
			reg_ra.subattempts = 0;
			reg_ra.startMWI();
			Receiver.registered();
		} else
			Receiver.onText(Receiver.REGISTER_NOTIFICATION+i, null, 0,0);
		if (wl[i].isHeld()) {
			wl[i].release();
			if (pwl[i] != null && pwl[i].isHeld()) pwl[i].release();
		}
	}

	String[] lastmsgs;
	
    public void onMWIUpdate(RegisterAgent mwi_ra, boolean voicemail, int number, String vmacc) {
    	int i = 0;
    	for (RegisterAgent ra : ras) {
    		if (ra == mwi_ra) break;
    		i++;
    	}
    	if (i != pref) return;
		if (voicemail) {
			String msgs = getUIContext().getString(R.string.voicemail);
			if (number != 0) {
				msgs = msgs + ": " + number;
			}
			Receiver.MWI_account = vmacc;
			if (lastmsgs[i] == null || !msgs.equals(lastmsgs[i])) {
				Receiver.onText(Receiver.MWI_NOTIFICATION, msgs,android.R.drawable.stat_notify_voicemail,0);
				lastmsgs[i] = msgs;
			}
		} else {
			Receiver.onText(Receiver.MWI_NOTIFICATION, null, 0,0);
			lastmsgs[i] = null;
		}
	}

	static long lasthalt,lastpwl;
	
	/** When a UA failed on (un)registering. */
	public void onUaRegistrationFailure(RegisterAgent reg_ra, NameAddress target,
			NameAddress contact, String result) {
		boolean retry = false;
    	int i = 0;
    	for (RegisterAgent ra : ras) {
    		if (ra == reg_ra) break;
    		i++;
    	}
    	if (isRegistered(i)) {
    		reg_ra.CurrentState = RegisterAgent.UNREGISTERED;
    		Receiver.onText(Receiver.REGISTER_NOTIFICATION+i, null, 0, 0);
    	} else {
    		retry = true;
    		Receiver.onText(Receiver.REGISTER_NOTIFICATION+i,getUIContext().getString(R.string.regfailed)+" ("+result+")",R.drawable.sym_presence_away,0);
    	}
    	if (retry && SystemClock.uptimeMillis() > lastpwl + 45000 && pwl[i] != null && !pwl[i].isHeld() && Receiver.on_wlan) {
			lastpwl = SystemClock.uptimeMillis();
			if (wl[i].isHeld())
				wl[i].release();
			pwl[i].acquire();
			register();
			if (!wl[i].isHeld() && pwl[i].isHeld()) pwl[i].release();
		} else if (wl[i].isHeld()) {
			wl[i].release();
			if (pwl[i] != null && pwl[i].isHeld()) pwl[i].release();
		}
		if (SystemClock.uptimeMillis() > lasthalt + 45000) {
			lasthalt = SystemClock.uptimeMillis();
			sip_providers[i].haltConnections();
		}
		/* if (!Thread.currentThread().getName().equals("main"))
			updateDNS(); */
		reg_ra.stopMWI();
    	WifiManager wm = (WifiManager) Receiver.mContext.getSystemService(Context.WIFI_SERVICE);
    	wm.startScan();
	}
	
	/* public void updateDNS() {
		Editor edit = PreferenceManager.getDefaultSharedPreferences(getUIContext()).edit();
		int i = 0;
		for (SipProvider sip_provider : sip_providers) {
			try {
				edit.putString(Settings.PREF_DNS+i, IpAddress.getByName(PreferenceManager.getDefaultSharedPreferences(getUIContext()).getString(Settings.PREF_SERVER+(i!=0?i:""), "")).toString());
			} catch (UnknownHostException e1) {
				i++;
				continue;
			}
			edit.commit();
			setOutboundProxy(sip_provider,i);
			i++;
		}
	} */

	/** Receives incoming calls (auto accept) */
	public void listen() 
	{
		for (UserAgent ua : uas) {
			if (ua != null) {
				ua.printLog("UAS: WAITING FOR INCOMING CALL");
				
				if (!ua.user_profile.audio && !ua.user_profile.video)
				{
					ua.printLog("ONLY SIGNALING, NO MEDIA");
				}
				
				ua.listen();
			}
		}
	}
	
	public void info(char c, int duration) {
		ua.info(c, duration);
	}
	
	/** Makes a new call */
	public boolean call(String target_url,boolean force) {
		int p = pref;
		
		boolean found = false;
		
		if (isRegistered(p) && Receiver.isFast(p))
			found = true;
		else {
			for (p = 0; p < lineCount; p++)
				if (isRegistered(p) && Receiver.isFast(p)) {
					found = true;
					break;
				}
			if (!found && force) {
				p = pref;
				if (Receiver.isFast(p))
					found = true;
				else for (p = 0; p < lineCount; p++)
					if (Receiver.isFast(p)) {
						found = true;
						break;
					}
			}
		}
				
		if (!found)
			return false;
		
		return call(target_url, force, p);
	}
	
	String lastError = null;
	public String getLastError(boolean clear) {
		String result = lastError;
		if(clear)
			lastError = null;
		return result;
	}
	
	public boolean call(DialCandidate target, boolean force) {
		lastError = null;
		if(!target.getScheme().equals("sip")) {
			lastError = "can't call non-SIP candidate";
			if(ua != null)
				ua.printLog(lastError);
			return false;
		}
		
		String target_url = target.getAddress();
		
		SIPIdentity sipIdentity = DialCandidateHelper.getSIPIdentity(this.getUIContext(), target);
		if(sipIdentity == null) {
			lastError = "no SIP Identity found or no default SIP identity set in preferences";
			if(ua != null)
				ua.printLog(lastError);
			return false;
		}
		
		int p = 0;
		for(p = 0; p < lineCount; p++) {
			if(user_profiles[p].sipIdentityId == sipIdentity.getId())
				return call(target_url, force, p);
		}
		
		lastError = "SIP identity not active, can't make call";
		if(ua != null)
			ua.printLog(lastError);
		return false;
	}
	
	public boolean sendMessage(SIPIdentity sender, String recipient, String body, MessageSendingRequest msr) {
		int p = 0;
		for(p = 0; p < lineCount; p++) {
			if(user_profiles[p].sipIdentityId == sender.getId()) {
				MessageAgent ma = mas[p];
				ma.send(recipient, "", body, msr);
				return true;
			}
		}
		
		lastError = "Failed to find profile for SIP identity: " + sender.getId();
		if(ua != null)
			ua.printLog(lastError);
		return false;
	}
	
	public boolean call(String target_url, boolean force, int p) {
		
		if((ua = uas[p]) == null) {
			/* if (PreferenceManager.getDefaultSharedPreferences(getUIContext()).getBoolean(Settings.PREF_CALLBACK, Settings.DEFAULT_CALLBACK) &&
					PreferenceManager.getDefaultSharedPreferences(getUIContext()).getString(Settings.PREF_POSURL, Settings.DEFAULT_POSURL).length() > 0) {
				Receiver.url("n="+Uri.decode(target_url));
				return true;
			} */
			return false;
		}

		ua.printLog("UAC: CALLING " + target_url);
		
		if (!ua.user_profile.audio && !ua.user_profile.video)
		{
			 ua.printLog("ONLY SIGNALING, NO MEDIA");
		}
		return ua.call(target_url, false);
	}

	public void answercall() 
	{
		Receiver.stopRingtone();
		ua.accept();
	}

	public void rejectcall() {
		ua.printLog("UA: HANGUP");
		ua.hangup();
	}

	public void togglehold() {
		ua.reInvite(null, 0);
	}

	public void transfer(String number) {
		ua.callTransfer(number, 0);
	}
	
	public void togglemute() {
		if (ua.muteMediaApplication())
			Receiver.onText(Receiver.CALL_NOTIFICATION, getUIContext().getString(R.string.menu_mute), android.R.drawable.stat_notify_call_mute,Receiver.ccCall.base);
		else
			Receiver.progress();
	}
	
	public void togglebluetooth() {
		ua.bluetoothMediaApplication();
		Receiver.progress();
	}
	
	public int speaker(int mode) {
		int ret = ua.speakerMediaApplication(mode);
		Receiver.progress();
		return ret;
	}
	
	public void keepAlive() {
		int i = 0;
		for (KeepAliveSip ka : kas) {
			if (ka != null && Receiver.on_wlan && isRegistered(i))
				try {
					ka.sendToken();
					Receiver.alarm(60, LoopAlarm.class);
				} catch (IOException e) {
					if (!Sipdroid.release) e.printStackTrace();
				}
			i++;
		}
	}
}
