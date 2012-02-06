package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;

public class SIPCarrierCandidateHarvester implements DialCandidateHarvester {
	
	public final static String SOURCE_INFO = "VoIP Carrier";

	@Override
	public List<DialCandidate> getCandidatesForNumber(Context context,
			String dialedNumber, String e164Number) {
		
		List<DialCandidate> candidates = new Vector<DialCandidate>();
		
		String number = dialedNumber;
		boolean usePrefix = false;
		if(e164Number != null) {
			number = e164Number.substring(1);  // Strip off the leading +
			usePrefix = true;
		}
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		
		List<SIPIdentity> sipIdentities = ds.getSIPIdentities();
		
		for(SIPIdentity s : sipIdentities) {
			String prefix = s.getCarrierIntlPrefix();
			String uri = s.getUri();
			String domain = uri.substring(uri.indexOf('@') + 1);
			if(s.isCarrierRoute()) {
				String sipAddress = ((usePrefix && prefix != null) ? prefix : "")
						+ number
						+ "@" + domain;
				candidates.add(new DialCandidate("sip", sipAddress, "", SOURCE_INFO, s));
			}
		}
		
		ds.close();
		
		return candidates;
	}

}
