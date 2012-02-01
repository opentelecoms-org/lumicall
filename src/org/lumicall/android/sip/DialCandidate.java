package org.lumicall.android.sip;

import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;

public class DialCandidate implements Parcelable {
	String scheme;
	String address;
	String displayName;
	String source;
	DialCandidate(String scheme, String address, String displayName, String source) {
		this.scheme = scheme;
		this.address = address;
		this.displayName = displayName;
		this.source = source;
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
	public String getDomain() {
		int i = address.indexOf('@');
		if(i == -1)
			return null;
		return address.substring(i + 1);
	}
	@Override
	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(scheme);
		dest.writeString(address);
		dest.writeString(displayName);
		dest.writeString(source);
	}
	private DialCandidate(Parcel source) {
		scheme = source.readString();
		address = source.readString();
		displayName = source.readString();
		this.source = source.readString();
	}

	public static final Parcelable.Creator<DialCandidate> CREATOR = new Parcelable.Creator<DialCandidate>() {
		public DialCandidate createFromParcel(Parcel in) {
			return new DialCandidate(in);
		}

		public DialCandidate[] newArray(int size) {
			return new DialCandidate[size];
		}
	};
	public String getAddressToDial() {
		return scheme + ":" + address;
	}

}
