package org.lumicall.android.preferences;

import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.DBObject;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.Button;
import android.widget.ListView;

public abstract class DBObjectsSettings<T extends DBObject> extends PreferenceActivity {
	
	protected abstract Class getEditorActivity();
	
	private class PreferenceWrapper<T extends DBObject> extends Preference implements Preference.OnPreferenceClickListener {

		T object;
		public PreferenceWrapper(Context context, T object) {
			super(context);
			this.object = object;
			setTitle(object.getTitleForMenu());
			setOnPreferenceClickListener(this);
		}
		@Override
		public boolean onPreferenceClick(Preference preference) {
			final Intent intent = new Intent(DBObjectsSettings.this, getEditorActivity());
            intent.putExtra(object.getKeyForIntent(), object.getId());
            startActivity(intent);
            finish();
			return true;
		}
		
		public T getObject() {
			return object;
		}
	}
	
	protected abstract List<T> getObjects(LumicallDataSource ds);
	
	private void loadPreferences(PreferenceScreen ps) {
		ps.setOrderingAsAdded(true);
		Context context = ps.getContext();
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		
		List<T> objects = getObjects(ds);
		
		for(T object : objects) {
			PreferenceWrapper<T> pw = new PreferenceWrapper<T>(context, object);
			ps.addPreference(pw);
		}
		
		ds.close();
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.xml.db_objects_settings);

		loadPreferences(getPreferenceScreen());
		
		registerForContextMenu(getListView());
		
		ListView v = getListView();
		Button addButton = new Button(this);
		addButton.setText(R.string.add);
		addButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                addObject();
            }
        });
        v.addFooterView(addButton);
	}
	
	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.db_objects_menu, menu);
        return true;
    }
	
	protected abstract String getKeyForIntent();

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        case R.id.add_db_object:
            addObject();
            return true; 
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.db_objects_context_menu, menu);
    }
    
    protected abstract void deleteObject(LumicallDataSource ds, T object);
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item
				.getMenuInfo();
		PreferenceWrapper<T> pw = (PreferenceWrapper<T>) this.getPreferenceScreen().getPreference(info.position);
		T object = pw.getObject();

		switch (item.getItemId()) {
		case R.id.delete_db_object:
			LumicallDataSource ds = new LumicallDataSource(pw.getContext());
			ds.open();
			deleteObject(ds, object);
			ds.close();
			getPreferenceScreen().removePreference(pw);
			return true;
		default:
			return super.onContextItemSelected(item);
		}
	}
    
    private void addObject() {
    	final Intent intent = new Intent(DBObjectsSettings.this, getEditorActivity());
        intent.putExtra(getKeyForIntent(), Long.valueOf(-1));
        startActivity(intent);
        finish();
    }


}
