package org.lumicall.android.sip;

import java.util.Collection;
import java.util.HashMap;
import java.util.Vector;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.util.Log;

public class AndroidEmailCandidateHarvester extends EmailCandidateHarvester {
	
	Context context;
	
	public AndroidEmailCandidateHarvester(Context context) {
		this.context = context;
	}

	@Override
	protected Collection<DialCandidate> getEmailAddressesForNumber(String number) {
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
