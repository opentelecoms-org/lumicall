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
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

import org.lumicall.android.AppProperties;
import org.lumicall.android.db.ENUMSuffix;
import org.lumicall.android.db.LumicallDataSource;

import uk.nominet.DDDS.ENUM;
import uk.nominet.DDDS.Rule;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.SQLException;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.util.Log;

public class ENUMProviderForSIP extends ContentProvider {

	/* Log tag */
	static private final String TAG = "ENUMProviderForSIP";
	
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
		Log.d(TAG, "onCreate() called");
		// FIXME - should we have a hardcoded DNS server?
        //System.setProperty("dns.server", ROOT_SERVERS);
		return true;
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
	
	private static class LookupThread extends Thread {
		Rule[] rules;
		CyclicBarrier barrier;
		String suffix;
		String number;
		private ENUM mENUM = null;

		LookupThread(CyclicBarrier barrier, String number, String suffix) {
			this.barrier = barrier;
			rules = new Rule[] {};
			this.suffix = suffix;
			this.number = number;
		}

		public Rule[] getRules() {
			return rules;
		}

		public void run() {
			try {
				Log.v(TAG, "looking up " + number + " in " + suffix);
				mENUM = new ENUM(suffix);
				rules = mENUM.lookup(number);

				barrier.await();
			} catch (InterruptedException ex) {
				ex.printStackTrace();
			} catch (BrokenBarrierException ex) {
				ex.printStackTrace();
			}
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
			Log.e(TAG, "failed to load ENUM suffixes from DB", e);
			return null;
		}
		
		// Setup variables
		MatrixCursor c = new MatrixCursor(COLUMN_NAMES, 10);
		CyclicBarrier b = new CyclicBarrier(suffixes.size() + 1);
		Vector<LookupThread> v = new Vector<LookupThread>();
		
		// Run the lookups in parallel
		for(String suffix : suffixes) {
			LookupThread t = new LookupThread(b, number, suffix);
			v.add(t);
			t.start();
		}
		
		// Wait for all lookups to finish
		try {
			b.await();
		} catch (InterruptedException e) {
			Log.e(TAG, "InterruptedException", e);
		} catch (BrokenBarrierException e) {
			Log.e(TAG, "BrokenBarrierException", e);
		}
		
		// Process the results
		// Notice that we process them in the order of the suffixes
		// Earlier results should have priority over later results
		for(LookupThread t : v) {
			for (Rule rule : t.getRules()) {
				parseRule(rule, c);
			}
		}
		return c;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		return 0;
	}
}
