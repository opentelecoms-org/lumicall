/*
 * Copyright 2011 Daniel Pocock
 * 
 * Based on code from ENUMDroid (Copyright 2009 Nominet UK)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.opentelecoms.android.sip;

import uk.nominet.DDDS.ENUM;
import uk.nominet.DDDS.Rule;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.util.Log;

public class ENUMProviderForSIP extends ContentProvider {

	/* Log tag */
	static private final String TAG = "ENUMProviderForSIP";

	/* exported constants */
	static public final Uri CONTENT_URI = Uri.parse("content://enum/");
	// FIXME - we lose all the priority and weighting values
	// A more advanced implementation would use that data for
	// trying multiple routes before failing
	static public final String[] COLUMN_NAMES = {
		"_id", "service", "uri",
	};
	
	/* member variables */
	private ENUM mENUM = null;
	
	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {
		return 0;
	}

	@Override
	public String getType(Uri uri) {
		return null;
	}

	@Override
	public Uri insert(Uri uri, ContentValues values) {
		return null;
	}

	@Override
	public boolean onCreate() {
		Log.d(TAG, "onCreate() called");
		// FIXME - should we have a hardcoded DNS server?
        System.setProperty("dns.server", "208.67.222.222,208.67.220.220");
		return true;
	}

	private String getSuffix() {
		/* SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
		String suffix = null;
		if (prefs.getBoolean(ENUMPrefs.ENUM_PREF_CUSTOM, false)) {
			suffix = prefs.getString(ENUMPrefs.ENUM_PREF_SUFFIX, null);
		}
		if (suffix == null || suffix.length() <= 0) {
			suffix = "e164.arpa";
		}
		return suffix; */
		
		// FIXME - should not be hard-coded
		return "e164.lvdx.com";
	}
	
	protected void parseRule(Rule rule, MatrixCursor c)
	{
		// split service field on '+' token
		String[] services = rule.getService().toLowerCase().split("\\+");
		
		// check that resulting fields are valid
		if (services.length != 2) return;	// not x+y
		if (!services[0].equals("e2u")) return; // not E2U+...
		
		// Only parse SIP-compatible records, others are silently
		// ignored
		for(String s : services) {
			if(s.equals("sip")) {
				String result = rule.evaluate();
				if (result != null) {
					// record ID is just the current record count
					Integer id = new Integer(c.getCount());
					Object[] row = new Object[] { id, "sip", result };
					Log.v(TAG, "sip" + " -> " + result);
					c.addRow(row);
				}
			} 
		}
	}
	
	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
			String[] selectionArgs, String sortOrder)
	{
		String number = uri.getPath().substring(1);
		String suffix = getSuffix();
		Log.v(TAG, "looking up " + number + " in " + suffix);

		mENUM = new ENUM(suffix);
    	Rule[] rules = mENUM.lookup(number);
    	
		MatrixCursor c = new MatrixCursor(COLUMN_NAMES, 10);
		for (Rule rule : rules) {
			parseRule(rule, c);
		}
		
		return c;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		return 0;
	}
}
