package org.lumicall.android.preferences;

import java.util.List;

import org.lumicall.android.R;
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

public class SIPIdentitiesSettings extends PreferenceActivity {
	
	private class PreferenceWrapper extends Preference implements Preference.OnPreferenceClickListener {

		SIPIdentity sipIdentity;
		public PreferenceWrapper(Context context, SIPIdentity sipIdentity) {
			super(context);
			this.sipIdentity = sipIdentity;
			setTitle(sipIdentity.getUri());
			setOnPreferenceClickListener(this);
		}
		@Override
		public boolean onPreferenceClick(Preference preference) {
			final Intent intent = new Intent(SIPIdentitiesSettings.this, SIPIdentitySettings.class);
            intent.putExtra(SIPIdentitySettings.SIP_IDENTITY_ID, sipIdentity.getId());
            startActivity(intent);
            finish();
			return true;
		}
		
		public SIPIdentity getSIPIdentity() {
			return sipIdentity;
		}
	}
	
	private void loadPreferences(PreferenceScreen ps) {
		ps.setOrderingAsAdded(true);
		Context context = ps.getContext();
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		
		List<SIPIdentity> sipIdentities = ds.getSIPIdentities();
		
		for(SIPIdentity s : sipIdentities) {
			PreferenceWrapper pw = new PreferenceWrapper(context, s);
			ps.addPreference(pw);
		}
		
		ds.close();
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.xml.sip_identities_settings);

		loadPreferences(getPreferenceScreen());
		
		registerForContextMenu(getListView());
	}
	
	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.sip_identities_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        case R.id.add_sip_identity:
            final Intent intent = new Intent(SIPIdentitiesSettings.this, SIPIdentitySettings.class);
            intent.putExtra(SIPIdentitySettings.SIP_IDENTITY_ID, new Long(-1));
            startActivity(intent);
            finish();
            return true; 
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.sip_identities_context_menu, menu);
    }
    
    @Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item
				.getMenuInfo();
		PreferenceWrapper pw = (PreferenceWrapper) this.getPreferenceScreen().getPreference(info.position);
		SIPIdentity sipIdentity = pw.getSIPIdentity();

		switch (item.getItemId()) {
		case R.id.delete_sip_identity:
			LumicallDataSource ds = new LumicallDataSource(pw.getContext());
			ds.open();
			ds.deleteSIPIdentity(sipIdentity);
			ds.close();
			getPreferenceScreen().removePreference(pw);
			return true;
		default:
			return super.onContextItemSelected(item);
		}
	}


}
