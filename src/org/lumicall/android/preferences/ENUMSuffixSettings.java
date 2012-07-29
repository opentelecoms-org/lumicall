package org.lumicall.android.preferences;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.math.BigDecimal;
import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.ENUMSuffix;
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

public class ENUMSuffixSettings extends DBObjectSettings<ENUMSuffix> {
	
	@Override
	protected void persistObject(LumicallDataSource ds, ENUMSuffix object) {
		ds.persistENUMSuffix(object);
	}

	@Override
	protected ENUMSuffix createObject() {
		return new ENUMSuffix();
	}

	@Override
	protected ENUMSuffix getObject(LumicallDataSource ds, long id) {
		return ds.getENUMSuffix(id);
	}

	@Override
	protected String getKeyForIntent() {
		return ENUMSuffix.ENUM_SUFFIX_ID;
	}

	@Override
	protected int getResForSettings() {
		return R.xml.enum_suffix_settings;
	}

	@Override
	protected void onChanged() {
		//Receiver.engine(context).updateDNS();
   		//Receiver.engine(this.getBaseContext()).halt();
		//Receiver.engine(this.getBaseContext()).StartEngine();
	}

}
