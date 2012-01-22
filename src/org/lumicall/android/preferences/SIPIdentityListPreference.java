package org.lumicall.android.preferences;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;
import android.preference.ListPreference;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

public class SIPIdentityListPreference extends ListPreference {
	
	List<SIPIdentity> sipIdentities;

	public SIPIdentityListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		init(context);
	}
	public SIPIdentityListPreference(Context context) {
		super(context);
		init(context);
	}
	
	protected View onCreateDialogView() {
		init(this.getContext());
		return super.onCreateDialogView();
	}
	
	protected void init(Context context) {
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		sipIdentities = ds.getSIPIdentities();
		ds.close();
		
		Vector<CharSequence> entries = new Vector<CharSequence>();
		Vector<CharSequence> values = new Vector<CharSequence>();
		for(SIPIdentity s : sipIdentities) {
			entries.add(s.getUri());
			values.add(Long.toString(s.getId()));
		}
		
		setEntries(entries.toArray(new CharSequence[] {}));
		setEntryValues(values.toArray(new CharSequence[] {}));
		
	}
	
	

}
