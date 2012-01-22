package org.lumicall.android.preferences;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.math.BigDecimal;
import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.sipdroid.sipua.ui.Receiver;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.text.InputType;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.TextView;

public class SIPIdentitySettings extends PreferenceActivity {
	
	public final static String SIP_IDENTITY_ID = "sipIdentityId";
	
	SIPIdentity sipIdentity;
	
	boolean changed = false;
	
	/* private void setEditTextPreference(String preferenceName, String value) {
		EditTextPreference p = (EditTextPreference) findPreference(preferenceName);
        p.setText(value);
	}
	
	private void setNumericEditTextPreference(String preferenceName, int value) {
		EditTextPreference p = (EditTextPreference) findPreference(preferenceName);
        p.setText(new Integer(value).toString());
	}
	
	private void setFloatEditTextPreference(String preferenceName, float value) {
		EditTextPreference p = (EditTextPreference) findPreference(preferenceName);
        p.setText(new Float(value).toString());
	}
	
	private void setCheckBoxPreference(String preferenceName, boolean value) {
		CheckBoxPreference p = (CheckBoxPreference) findPreference(preferenceName);
		p.setChecked(value);
	}
	
	private void setListPreference(String preferenceName, String value) {
		ListPreference p = (ListPreference) findPreference(preferenceName);
		p.setValue(value);
	} */
	
	private static boolean isGetter(String s) {
		return s.substring(0, 2).equals("is") || s.substring(0, 3).equals("get");
	}
	
	private static boolean isSetter(String s) {
		return s.substring(0, 3).equals("set");
	}
	
	private void setPreferenceValue(Method getter, Preference p) throws IllegalArgumentException, IllegalAccessException, InvocationTargetException {
		Object ret = getter.invoke(sipIdentity, (Object[])null);
		if(ret != null) {
			if(p.getClass().equals(EditTextPreference.class)) {
				EditTextPreference etp = (EditTextPreference)p;
				etp.setText(ret.toString());
				if((etp.getEditText().getInputType() & InputType.TYPE_TEXT_VARIATION_PASSWORD) == 0)
					etp.setSummary(ret.toString());
			} else if(p.getClass().equals(CheckBoxPreference.class)) {
				((CheckBoxPreference)p).setChecked((Boolean)ret);
			} else if(p.getClass().equals(ListPreference.class)) {
				ListPreference lp = (ListPreference)p;
				lp.setValue(ret.toString());
				lp.setSummary(ret.toString());
			}
		}
	}
	
	private void updatePrefs() {

		try {
			Method[] methods = Class.forName(sipIdentity.getClass().getName())
					.getMethods();

			for (int i = 0; i < methods.length; i++) {
				PreferenceField preferenceField = methods[i]
						.getAnnotation(PreferenceField.class);
				if (isGetter(methods[i].getName()) && preferenceField != null) {
					String fieldName = preferenceField.fieldName();
					Preference p = findPreference(fieldName);
					setPreferenceValue(methods[i], p);
					p.setOnPreferenceChangeListener(new MyListener());
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		
        /* setEditTextPreference("sip_identity_uri", sipIdentity.getUri());
        setEditTextPreference("sip_identity_auth_user", sipIdentity.getAuthUser());
        setEditTextPreference("sip_identity_auth_password", sipIdentity.getAuthPassword());
        setCheckBoxPreference("sip_identity_registration", sipIdentity.isReg());
        setFloatEditTextPreference("sip_identity_q", sipIdentity.getQ());
        setEditTextPreference("sip_identity_reg_server_name", sipIdentity.getRegServerName());
        setNumericEditTextPreference("sip_identity_reg_server_port", sipIdentity.getRegServerPort());
        setListPreference("sip_identity_reg_server_protocol", sipIdentity.getRegServerProtocol());
        setCheckBoxPreference("sip_identity_outbound", sipIdentity.isOutbound());
        setEditTextPreference("sip_identity_outbound_server_name", sipIdentity.getOutboundServerName());
        setNumericEditTextPreference("sip_identity_outbound_server_port", sipIdentity.getOutboundServerPort());
        setListPreference("sip_identity_outbound_server_protocol", sipIdentity.getOutboundServerProtocol());
        setEditTextPreference("sip_identity_carrier_intl_prefix", sipIdentity.getCarrierIntlPrefix());
        setEditTextPreference("sip_identity_stun_server_name", sipIdentity.getStunServerName());
        setNumericEditTextPreference("sip_identity_stun_server_port", sipIdentity.getStunServerPort());
        setListPreference("sip_identity_stun_server_protocol", sipIdentity.getStunServerProtocol()); */
	}
	
	private boolean setBeanValue(Method m, Preference p, Object newValue) throws IllegalArgumentException, IllegalAccessException, InvocationTargetException {
		int hash1 = sipIdentity.hashCode();
		if(m.getParameterTypes()[0].equals(String.class)) {
			m.invoke(sipIdentity, newValue);
		} else if(m.getParameterTypes()[0].equals(Float.TYPE)) {
			m.invoke(sipIdentity, new Float(new BigDecimal((String)newValue).floatValue()));
		} else if(m.getParameterTypes()[0].equals(Boolean.TYPE)) {
			m.invoke(sipIdentity, newValue);
		} else {
			Log.e(getClass().getName(), "unhandled type: " + m.getParameterTypes()[0].getName());
		}
		
		LumicallDataSource ds = new LumicallDataSource(p.getContext());
		ds.open();
		ds.persistSIPIdentity(sipIdentity);
		ds.close();
		
		int hash2 = sipIdentity.hashCode();
		return (hash1 != hash2);
	}
	
	private class MyListener implements Preference.OnPreferenceChangeListener {
		
		@Override
		public boolean onPreferenceChange(Preference preference, Object newValue) {
			String key = preference.getKey();
			boolean _changed = false;
			try {
				Method[] methods = Class.forName(
						sipIdentity.getClass().getName()).getMethods();

				for (int i = 0; i < methods.length; i++) {
					PreferenceField preferenceField = methods[i]
							.getAnnotation(PreferenceField.class);
					if (isSetter(methods[i].getName())
							&& preferenceField != null
							&& preferenceField.fieldName().equals(key)) {
						_changed = setBeanValue(methods[i], preference, newValue);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			if(_changed)
				changed = true;
			return true;
		}
	}
	
	private void loadPreferences(PreferenceScreen ps, long id) {
		Context context = ps.getContext();
		
		if(id == -1) {
			sipIdentity = new SIPIdentity();
		} else {
			LumicallDataSource ds = new LumicallDataSource(context);
			ds.open();
		
			sipIdentity = ds.getSIPIdentity(id);
			
			ds.close();
		}
		updatePrefs();
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.xml.sip_identity_settings);
		
		Bundle extras = getIntent().getExtras();

		long id = -1;
        if (extras != null) {
                id = extras.getLong(SIP_IDENTITY_ID);
        }

		loadPreferences(getPreferenceScreen(), id);
	}
	
	protected void onStop() {
		checkForChanges();
		super.onStop();
	}
	
	protected void checkForChanges() {
		if(changed) {
			//Receiver.engine(context).updateDNS();
	   		Receiver.engine(this.getBaseContext()).halt();
			Receiver.engine(this.getBaseContext()).StartEngine();
			changed = false;
		}
	}

}
