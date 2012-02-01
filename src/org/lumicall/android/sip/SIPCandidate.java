package org.lumicall.android.sip;

import android.net.Uri;

public class SIPCandidate {
	String address;
	String displayName;
	String source;
	SIPCandidate(String address, String displayName, String source) {
		this.address = address;
		this.displayName = displayName;
		this.source = source;
	}
	public String getAddress() {
		return address;
	}
	public String getDisplayName() {
		return displayName;
	}
	public String getSource() {
		return source;
	}
	public String getDomain() {
		int i = address.indexOf('@');
		if(i == -1)
			return null;
		return address.substring(i + 1);
	}
}
