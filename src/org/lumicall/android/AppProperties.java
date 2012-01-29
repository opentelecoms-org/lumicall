package org.lumicall.android;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;

public class AppProperties extends Properties {
	
	private final static String FILENAME = "app.properties";
	
	protected final static String REGISTRATION_URL = "registrationUrl";
	protected final static String SIP_SERVER = "sipServer";
	protected final static String SIP_DOMAIN = "sipDomain";
	protected final static String SIP_PROTOCOL = "sipProtocol";
	protected final static String SIP_PORT = "sipPort";
	protected final static String STUN_SERVER = "stunServer";
	protected final static String STUN_PORT = "stunPort";

	public AppProperties(Context context) throws IOException {
		Resources resources = context.getResources();
		AssetManager assetManager = resources.getAssets();

		// Read from the /assets directory
		InputStream inputStream = assetManager.open(FILENAME);
		load(inputStream);
		
	}
	
	public String getRegistrationUrl() {
		return getProperty(REGISTRATION_URL);
	}
	
	public String getSipServer() {
		return getProperty(SIP_SERVER);
	}
	
	public String getSipDomain() {
		return getProperty(SIP_DOMAIN);
	}
	
	public String getSipProtocol() {
		return getProperty(SIP_PROTOCOL);
	}
	
	public int getSipPort() {
		return Integer.parseInt(getProperty(SIP_PORT));
	}
	
	public String getStunServer() {
		return getProperty(STUN_SERVER);
	}
	
	public int getStunPort() {
		return Integer.parseInt(getProperty(STUN_PORT));
	}

}
