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

package org.lumicall.android.sip;

import java.io.IOException;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;

import org.lumicall.android.db.ENUMSuffix;
import org.lumicall.android.db.LumicallDataSource;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.SQLException;
import android.net.Uri;
import android.preference.PreferenceManager;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ENUMProviderForSIP extends ContentProvider {

	private static final Logger logger = LoggerFactory.getLogger(ENUMProviderForSIP.class);
	
	// These are the DNS servers - they will be tried by the DNS library, as well as trying resolv.conf and other methods
	public final static String DNS_SERVERS = "195.8.117.5";

	/* exported constants */
	static public final Uri CONTENT_URI = Uri.parse("content://enum/");
	// FIXME - we lose all the priority and weighting values
	// A more advanced implementation would use that data for
	// trying multiple routes before failing
	static public final String[] COLUMN_NAMES = {
		"_id", "service", "uri",
	};
	
	
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
		logger.debug("onCreate() called");
		// FIXME - should we have a hardcoded DNS server?
        //System.setProperty("dns.server", ROOT_SERVERS);
		return true;
	}
	
	protected void fillCursor(List<ENUMResult> results, MatrixCursor c) {
		for(ENUMResult result : results) {
			Integer id = new Integer(c.getCount());
			Object[] row = new Object[] { id, "sip", result.getResult() };
			logger.debug("sip" + " -> " + result);
			c.addRow(row);
		}
	}
	
	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
			String[] selectionArgs, String sortOrder) {
		String number = uri.getPath().substring(1);
		List<String> suffixes = new Vector<String>();
		try {
			LumicallDataSource ds = new LumicallDataSource(getContext());
			ds.open();
			List<ENUMSuffix> _suffixes = ds.getENUMSuffixes();
			ds.close();
			for(ENUMSuffix s : _suffixes) {
				suffixes.add(s.getSuffix());
			}
		} catch (SQLException e) {
			logger.warn("failed to load ENUM suffixes from DB", e);
			return null;
		}
		
		// Setup variables
		MatrixCursor c = new MatrixCursor(COLUMN_NAMES, 10);
		ENUMClient client = new ENUMClient(suffixes);
		
		List<ENUMResult> results = client.query(number);
		
		// Process the results
		// Notice that we process them in the order of the suffixes
		// Earlier results should have priority over later results
		fillCursor(results, c);
		return c;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		return 0;
	}
}
