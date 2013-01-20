package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;
import java.util.logging.Logger;

import org.sipdroid.sipua.ui.Receiver;
import org.zoolu.sip.dialog.OptionsDialog;
import org.zoolu.sip.dialog.OptionsDialogListener;
import org.zoolu.sip.message.Message;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

public class ENUMCandidateHarvester implements DialCandidateHarvester {
	
	Logger logger = Logger.getLogger(getClass().getCanonicalName());

	@Override
	public List<DialCandidate> getCandidatesForNumber(Context context,
			String number, String e164Number) {
		
		List<DialCandidate> candidates = new Vector<DialCandidate>();
		
		boolean online = ENUMUtil.updateNotification(context);
		
		if(!online) {
			logger.info("ENUM not online");
			return candidates;
		}
		
		if(e164Number == null || !e164Number.startsWith("+")) {
			logger.info("can't handle non-E.164 numbers");
			return candidates;
		}
		
		/* ask the ENUM ContentProvider for the records */
		Uri uri = Uri.withAppendedPath(ENUMProviderForSIP.CONTENT_URI, e164Number);
		ContentResolver cr = context.getContentResolver();
		Cursor mCursor = cr.query(uri, null, null, null, null);
		
		/* none found - tell the user then dial the original number */
		if (mCursor == null || mCursor.getCount() <= 0) {
			if(mCursor != null)
				mCursor.close();
			logger.info("no ENUM result found");
			return candidates;
		}

		while(mCursor.moveToNext()) {
			String destination = mCursor.getString(2);
			// Prevent prefix sip: from appearing twice (we add it again later)
			if(destination.startsWith("sip:"))
				destination = destination.substring(4);
			candidates.add(new DialCandidate("sip", destination, "", "ENUM"));
		}
		mCursor.close();
		
		logger.info("ENUM results found, count = " + candidates.size());
		
		return candidates;
	}

}
