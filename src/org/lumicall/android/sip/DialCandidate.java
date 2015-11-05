package org.lumicall.android.sip;

import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.sipdroid.sipua.ui.PSTN;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.widget.Toast;

public class DialCandidate implements Parcelable {
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
	public DialCandidate(String scheme, String address, String displayName, String source, SIPIdentity sipIdentity) {
		this.scheme = scheme;
		this.address = address;
		this.displayName = displayName;
		this.source = source;
		if(sipIdentity != null)
			this.sipIdentityId = sipIdentity.getId();
		else
			this.sipIdentityId = -1;
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
			_sipIdentityId = Long.parseLong(sipSettings.getString(Settings.PREF_SIP, "-1"));
		}
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		SIPIdentity sipIdentity = null;
		if(_sipIdentityId >= 0) {
			sipIdentity = ds.getSIPIdentity(_sipIdentityId);
		}
		if(sipIdentity == null) {
			// No default SIP identity selected in the prefs, just use the first one
			for(SIPIdentity _sipIdentity : ds.getSIPIdentities()) {
				if(_sipIdentity.isEnable()) {
					sipIdentity = _sipIdentity;
					break;
				}
			}
		}
		ds.close();
		if(sipIdentity == null || !sipIdentity.isEnable())
			return null;
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
	
	@Override
	public String toString() {
		return getAddressToDial() + " (" + getSource() + ")";
	}
	
	public boolean call(Context context) {
		if(getScheme().equals("sip")) {
			if(!Receiver.engine(context).call(this, true)) {
				String error = Receiver.engine(context).getLastError(true);
				if(error == null)
					error = context.getString(R.string.call_unknown_error);
				new AlertDialog.Builder(context)
					.setMessage(error)
					.setTitle(R.string.app_name)
					.setIcon(R.drawable.icon22)
					.setCancelable(true)
					.show();
				return false;
			} else {
				return true;
			}
		} else if(getScheme().equals("tel")) {
			PSTN.callPSTN2("tel:" + getAddress());
			return true;
		}
		return false;
	}

}
