package org.lumicall.android.sip;

import java.util.HashMap;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

import org.xbill.DNS.ExtendedResolver;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.Resolver;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.util.Log;

public class EmailCandidateHarvester implements DialCandidateHarvester {
	
	public EmailCandidateHarvester() {
		//System.setProperty("dns.server", ENUMProviderForSIP.ROOT_SERVERS);
	}
	
	final static String TAG = "EmailHarvester";
	
	private static class LookupThread extends Thread {
		
		CyclicBarrier barrier;
		DialCandidate sipCandidate;
		String key;
		int recordCount = 0;
		
		LookupThread(CyclicBarrier barrier, DialCandidate sipCandidate, String key) {
			this.barrier = barrier;
			this.sipCandidate = sipCandidate;
			this.key = key;
		}
		
		public DialCandidate getSIPCandidate() {
			return sipCandidate;
		}

		public int getRecordCount() {
			return recordCount;
		}

		public void run() {
			try {
				Log.v(TAG, "looking up " + key);

				try {
					Resolver resolver = new ExtendedResolver();
					resolver.setTimeout(2);
					Lookup lookup = new Lookup(key, Type.SRV);
					lookup.setResolver(resolver);
					Record[] records = lookup.run();

					if (records != null) {

						for (int i = 0; i < records.length; ++i) {
							// type check necessary in case of other RRtypes in the
							// Answer
							// Section
							if (records[i] instanceof SRVRecord) {
								recordCount++;
							}
						}
						Log.v(TAG, "found " + records.length + " records, SRV count = " + recordCount);
					} else {
						Log.v(TAG, "records == null, result = " + lookup.getErrorString());
					}
				} catch (Exception ex) {
					Log.w(TAG, "Exception during DNS lookup: ", ex);
				}
				barrier.await();
			} catch (InterruptedException ex) {
				ex.printStackTrace();
			} catch (BrokenBarrierException ex) {
				ex.printStackTrace();
			}
		}
	}

	public List<DialCandidate> getCandidatesForNumber(Context context, String number, String e164Number) {
		List<DialCandidate> candidates = getEmailAddressesForNumber(context, e164Number);
		Vector<DialCandidate> goodCandidates = new Vector<DialCandidate>();
		
		CyclicBarrier b = new CyclicBarrier(candidates.size() + 1);
		Vector<LookupThread> v = new Vector<LookupThread>();
		
		for(DialCandidate c : candidates) {
			// For each candidate, check if SRV record exists
			// We should check NAPTR too
			String key = "_sips._tcp." + c.getDomain() + ".";  // trailing dot required
			LookupThread t = new LookupThread(b, c, key);
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
			if(t.getRecordCount() > 0)
				goodCandidates.add(t.getSIPCandidate());
		}

		return goodCandidates;
	}
	
	private List<DialCandidate> getEmailAddressesForNumber(Context context, String number) {
		Vector<DialCandidate> v = new Vector<DialCandidate>();
		String SOURCE_NAME = "Contacts"; 
		
		// First identify the person (or list of people) with this number
		Uri contactRef = Uri.withAppendedPath(ContactsContract.PhoneLookup.CONTENT_FILTER_URI, number);
		Cursor personCursor = context.getContentResolver().query(contactRef, null, null, null, null);
		if(personCursor == null) {
			Log.v("Caller", "Number does not belong to a known contact");
			return v;
		}
		HashMap<String, String> person = new HashMap<String, String>();
		while(personCursor.moveToNext()) {
			String personId = personCursor.getString(personCursor.getColumnIndex(ContactsContract.PhoneLookup.LOOKUP_KEY));
			String name = personCursor.getString(personCursor.getColumnIndex(ContactsContract.PhoneLookup.DISPLAY_NAME));
			person.put(personId, name);
			Log.v("Caller", "ID = " + personId + ", name = " + name);
		}
		personCursor.close();
		Log.v("Caller", "Number of contacts with this number = " + person.size());
		
		for(String personId : person.keySet()) {
			Uri emailRef = ContactsContract.Data.CONTENT_URI;
			
			Cursor emailsCursor = context.getContentResolver().query(emailRef, null, 
				ContactsContract.Data.LOOKUP_KEY + " = ?", new String[]{personId}, null);

			if (emailsCursor != null) {
				while(emailsCursor.moveToNext()) {
					String rowType = emailsCursor.getString(
							emailsCursor.getColumnIndex(ContactsContract.Data.MIMETYPE));
					if(rowType.equals(Email.CONTENT_ITEM_TYPE)) {	
						String emailAddress = emailsCursor.getString(
							emailsCursor.getColumnIndex(ContactsContract.Data.DATA1));
						v.add(new DialCandidate("sip", emailAddress, 
								person.get(personId),
								SOURCE_NAME));
						Log.v("Caller", "found email address for contact: " + emailAddress);
					}
				}
				emailsCursor.close();
			}
		}
		return v;
	}
	
	
}
