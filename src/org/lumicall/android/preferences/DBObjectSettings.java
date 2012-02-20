package org.lumicall.android.preferences;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.math.BigDecimal;
import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.DBObject;
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

public abstract class DBObjectSettings<T extends DBObject> extends PreferenceActivity {
	
	T object;
	
	boolean changed = false;
	
	private static boolean isGetter(String s) {
		return s.substring(0, 2).equals("is") || s.substring(0, 3).equals("get");
	}
	
	private static boolean isSetter(String s) {
		return s.substring(0, 3).equals("set");
	}
	
	private void setPreferenceValue(Method getter, Preference p) throws IllegalArgumentException, IllegalAccessException, InvocationTargetException {
		Object ret = getter.invoke(object, (Object[])null);
		if(ret != null) {
			if(p.getClass().equals(EditTextPreference.class)) {
				EditTextPreference etp = (EditTextPreference)p;
				etp.setText(ret.toString());
				if((etp.getEditText().getInputType() & InputType.TYPE_TEXT_VARIATION_PASSWORD) == 0)
					updatePreferenceSummary(etp, ret.toString());
			} else if(p.getClass().equals(CheckBoxPreference.class)) {
				((CheckBoxPreference)p).setChecked((Boolean)ret);
			} else if(p.getClass().equals(ListPreference.class)) {
				ListPreference lp = (ListPreference)p;
				lp.setValue(ret.toString());
				updatePreferenceSummary(lp, ret.toString());
			}
		}
	}
	
	private void updatePrefs() {

		try {
			Method[] methods = Class.forName(object.getClass().getName())
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
	}
	
	protected abstract void persistObject(LumicallDataSource ds, T object);
	
	protected boolean setBeanValue(Method m, Preference p, Object newValue) throws IllegalArgumentException, IllegalAccessException, InvocationTargetException, SecurityException, NoSuchMethodException {
		int hash1 = object.hashCode();
		if(m.getParameterTypes()[0].equals(String.class)) {
			m.invoke(object, newValue);
		} else if(m.getParameterTypes()[0].equals(Float.TYPE)) {
			m.invoke(object, new Float(new BigDecimal((String)newValue).floatValue()));
		} else if(m.getParameterTypes()[0].equals(Integer.TYPE)) {
			m.invoke(object, new Integer((String)newValue).intValue());	
		} else if(m.getParameterTypes()[0].equals(Boolean.TYPE)) {
			m.invoke(object, newValue);
		} else if(m.getParameterTypes()[0].isEnum()) {
			Object _newValue = m.getParameterTypes()[0].getMethod("valueOf", String.class).invoke(null, newValue);
			m.invoke(object, _newValue);
		} else {
			Log.e(getClass().getName(), "unhandled type: " + m.getParameterTypes()[0].getName());
		}
		
		LumicallDataSource ds = new LumicallDataSource(p.getContext());
		ds.open();
		persistObject(ds, object);
		ds.close();
		
		int hash2 = object.hashCode();
		return (hash1 != hash2);
	}
	
	protected void updatePreferenceSummary(Preference p, String value) {
		p.setSummary(value);
	}
	
	private class MyListener implements Preference.OnPreferenceChangeListener {
		
		@Override
		public boolean onPreferenceChange(Preference preference, Object newValue) {
			String key = preference.getKey();
			boolean _changed = false;
			try {
				Method[] methods = Class.forName(
						object.getClass().getName()).getMethods();

				for (int i = 0; i < methods.length; i++) {
					PreferenceField preferenceField = methods[i]
							.getAnnotation(PreferenceField.class);
					if (isSetter(methods[i].getName())
							&& preferenceField != null
							&& preferenceField.fieldName().equals(key)) {
						_changed = setBeanValue(methods[i], preference, newValue);
					}
					if(!(preference instanceof CheckBoxPreference)) {
						updatePreferenceSummary(preference, newValue.toString());
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
	
	protected abstract T createObject();
	protected abstract T getObject(LumicallDataSource ds, long id);
	
	private void loadPreferences(PreferenceScreen ps, long id) {
		Context context = ps.getContext();
		
		if(id == -1) {
			object = createObject();
		} else {
			LumicallDataSource ds = new LumicallDataSource(context);
			ds.open();
		
			object = getObject(ds, id);
			
			ds.close();
		}
		updatePrefs();
	}
	
	protected abstract String getKeyForIntent();
	protected abstract int getResForSettings();
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(getResForSettings());
		
		Bundle extras = getIntent().getExtras();

		long id = -1;
        if (extras != null) {
                id = extras.getLong(getKeyForIntent());
        }

		loadPreferences(getPreferenceScreen(), id);
	}
	
	protected void onStop() {
		checkForChanges();
		super.onStop();
	}
	
	protected abstract void onChanged();
	
	protected void checkForChanges() {
		if(changed) {
			onChanged();
			changed = false;
		}
	}

}
