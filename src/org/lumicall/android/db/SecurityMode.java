package org.lumicall.android.db;

public enum SecurityMode {
	
	NONE ( "NONE" ),
	SRTP ( "SRTP" ),
	ZRTP ( "ZRTP" );
	
	String name;
	
	SecurityMode(String name) {
		this.name = name;
	}
	
	public String toString() {
		return name;
	}
	
}
