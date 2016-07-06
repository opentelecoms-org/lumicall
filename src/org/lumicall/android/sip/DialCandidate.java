package org.lumicall.android.sip;

import java.util.List;

public class DialCandidate {
	String scheme;
	String address;
	String displayName;
	String source;
	long sipIdentityId;
	public DialCandidate(String scheme, String address, String displayName, String source) {
		this.scheme = scheme;
		this.address = address;
		this.displayName = displayName;
		this.source = source;
		this.sipIdentityId = -1;
	}
	public DialCandidate(String scheme, String address, String displayName, String source, long sipIdentityId) {
		this.scheme = scheme;
		this.address = address;
		this.displayName = displayName;
		this.source = source;
		this.sipIdentityId = sipIdentityId;
	}
	public String getScheme() {
		return scheme;
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
	public long getSipIdentityId() {
		return sipIdentityId;
	}
	public String getDomain() {
		int i = address.indexOf('@');
		if(i == -1)
			return null;
		return address.substring(i + 1);
	}
	public String getAddressToDial() {
		return scheme + ":" + address;
	}
	
	@Override
	public String toString() {
		return getAddressToDial() + " (" + getSource() + ")";
	}
	
}
