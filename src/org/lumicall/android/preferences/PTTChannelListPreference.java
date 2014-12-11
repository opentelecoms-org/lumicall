package org.lumicall.android.preferences;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.PTTChannel;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;
import android.preference.ListPreference;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

public class PTTChannelListPreference extends DBObjectListPreference<PTTChannel> {
	
	public PTTChannelListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public PTTChannelListPreference(Context context) {
		super(context);
	}
	
	@Override
	protected List<PTTChannel> getObjects(LumicallDataSource ds) {
		return ds.getPTTChannels();
	}
}
