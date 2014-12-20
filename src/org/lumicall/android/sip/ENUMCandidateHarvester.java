package org.lumicall.android.sip;

import java.util.logging.Logger;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

public class ENUMCandidateHarvester extends DialCandidateHarvester implements Runnable {
	
	Logger logger = Logger.getLogger(getClass().getCanonicalName());
	private Context context;
	private String e164Number;
	
	public ENUMCandidateHarvester(Context context) {
		this.context = context;
	}

	@Override
	public void getCandidatesForNumber(String number, String e164Number) {
		
		boolean runHarvest = true;
		
		boolean online = ENUMUtil.updateNotification(context);
		
		if(!online) {
			logger.info("ENUM not online");
			runHarvest = false;
		}
		
		if(e164Number == null || !e164Number.startsWith("+")) {
			logger.info("can't handle non-E.164 numbers");
			runHarvest = false;
		}
		
		if(runHarvest) {
			this.e164Number = e164Number;
			Thread t = new Thread(this);
			t.start();
		} else {
			onHarvestCompletion();
		}
	}
	
	/**
	 * This is not ideal, it does all ENUM lookups in one thread and then
	 * returns all the results in one go.  It would be better to
	 * return each result as they arrive from DNS.
	 */
	@Override
	public void run() {
		/* ask the ENUM ContentProvider for the records */
		Uri uri = Uri.withAppendedPath(ENUMProviderForSIP.CONTENT_URI, e164Number);
		ContentResolver cr = context.getContentResolver();
		Cursor mCursor = cr.query(uri, null, null, null, null);
		
		/* none found - tell the user then dial the original number */
		if (mCursor == null || mCursor.getCount() <= 0) {
			if(mCursor != null)
				mCursor.close();
			logger.info("no ENUM result found");
			onHarvestCompletion();
			return;
		}

		int count = 0;
		while(mCursor.moveToNext()) {
			String destination = mCursor.getString(2);
			// Prevent prefix sip: from appearing twice (we add it again later)
			if(destination.startsWith("sip:"))
				destination = destination.substring(4);
			onDialCandidateFound(new DialCandidate("sip", destination, "", "ENUM"));
			count++;
		}
		mCursor.close();
		
		logger.info("ENUM results found, count = " + count);
		
		onHarvestCompletion();
	}

}
