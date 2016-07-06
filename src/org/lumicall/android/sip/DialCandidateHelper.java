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

import org.omnidial.harvest.DialCandidate;

public class DialCandidateHelper {
	public static SIPIdentity getSIPIdentity(Context context, DialCandidate dc) {
		long _sipIdentityId = dc.getSipIdentityId();
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
	
	public static boolean call(Context context, DialCandidate dc) {
		if(dc.getScheme().equals("sip")) {
			if(!Receiver.engine(context).call(dc, true)) {
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
		} else if(dc.getScheme().equals("tel")) {
			PSTN.callPSTN2("tel:" + dc.getAddress());
			return true;
		}
		return false;
	}

}
