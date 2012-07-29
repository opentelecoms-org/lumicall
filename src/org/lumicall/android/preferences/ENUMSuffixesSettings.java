package org.lumicall.android.preferences;

import java.util.List;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;

import org.lumicall.android.R;
import org.lumicall.android.db.ENUMSuffix;
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

public class ENUMSuffixesSettings extends DBObjectsSettings<ENUMSuffix> {

	@Override
	protected List<ENUMSuffix> getObjects(LumicallDataSource ds) {
		return ds.getENUMSuffixes();
	}

	@Override
	protected String getKeyForIntent() {
		return ENUMSuffix.ENUM_SUFFIX_ID;
	}

	@Override
	protected void deleteObject(LumicallDataSource ds, ENUMSuffix object) {
		ds.deleteENUMSuffix(object);
	}

	@Override
	protected Class getEditorActivity() {
		return ENUMSuffixSettings.class;
	}
}
