package org.lumicall.android.preferences;

import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.PTTChannel;
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

public class PTTChannelsSettings extends DBObjectsSettings<PTTChannel> {

	@Override
	protected List<PTTChannel> getObjects(LumicallDataSource ds) {
		return ds.getPTTChannels();
	}

	@Override
	protected String getKeyForIntent() {
		return PTTChannel.PTT_CHANNEL_ID;
	}

	@Override
	protected void deleteObject(LumicallDataSource ds, PTTChannel object) {
		ds.deletePTTChannel(object);
	}

	@Override
	protected Class getEditorActivity() {
		return PTTChannelSettings.class;
	}
}
