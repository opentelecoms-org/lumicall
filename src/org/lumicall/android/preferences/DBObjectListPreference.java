package org.lumicall.android.preferences;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.db.DBObject;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;
import android.preference.ListPreference;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

public abstract class DBObjectListPreference<T extends DBObject> extends ListPreference {
	
	List<T> objects;

	public DBObjectListPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		init(context);
	}
	public DBObjectListPreference(Context context) {
		super(context);
		init(context);
	}
	
	protected View onCreateDialogView() {
		init(this.getContext());
		return super.onCreateDialogView();
	}
	
	protected abstract List<T> getObjects(LumicallDataSource ds);
	
	protected void init(Context context) {
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		objects = getObjects(ds);
		ds.close();
		
		Vector<CharSequence> entries = new Vector<CharSequence>();
		Vector<CharSequence> values = new Vector<CharSequence>();
		for(T o : objects) {
			entries.add(o.getTitleForMenu());
			values.add(Long.toString(o.getId()));
		}
		
		setEntries(entries.toArray(new CharSequence[] {}));
		setEntryValues(values.toArray(new CharSequence[] {}));
		
	}
	
	

}
