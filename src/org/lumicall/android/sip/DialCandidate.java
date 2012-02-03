package org.lumicall.android.sip;

import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.sipdroid.sipua.ui.Settings;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;

public class DialCandidate implements Parcelable {
	String scheme;
	String address;
	String displayName;
	String source;
	long sipIdentityId;
	DialCandidate(String scheme, String address, String displayName, String source) {
		this.scheme = scheme;
		this.address = address;
		this.displayName = displayName;
		this.source = source;
		this.sipIdentityId = -1;
	}
	DialCandidate(String scheme, String address, String displayName, String source, SIPIdentity sipIdentity) {
		this.scheme = scheme;
		this.address = address;
		this.displayName = displayName;
		this.source = source;
		this.sipIdentityId = sipIdentity.getId();
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
	public SIPIdentity getSIPIdentity(Context context) {
		long _sipIdentityId = sipIdentityId;
		if(_sipIdentityId == -1) {
			// Must use the default SIP identity for SIP-SIP calls
			SharedPreferences sipSettings = context.getSharedPreferences(Settings.sharedPrefsFile, Context.MODE_PRIVATE);
			_sipIdentityId = sipSettings.getLong(Settings.PREF_SIP, -1);
		}
		if(_sipIdentityId == -1) {
			// No default SIP identity selected in the prefs
			return null;
		}
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		SIPIdentity sipIdentity = ds.getSIPIdentity(_sipIdentityId);
		ds.close();
		return sipIdentity;
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
		dest.writeLong(sipIdentityId);
	}
	private DialCandidate(Parcel source) {
		scheme = source.readString();
		address = source.readString();
		displayName = source.readString();
		this.source = source.readString();
		sipIdentityId = source.readLong();
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
