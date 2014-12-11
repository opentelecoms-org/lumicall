package org.lumicall.android.preferences;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;
import android.preference.ListPreference;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

public class SIPIdentityListPreference extends DBObjectListPreference<SIPIdentity> {
	
	public SIPIdentityListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public SIPIdentityListPreference(Context context) {
		super(context);
	}
	
	@Override
	protected List<SIPIdentity> getObjects(LumicallDataSource ds) {
		return ds.getSIPIdentities();
	}
}
