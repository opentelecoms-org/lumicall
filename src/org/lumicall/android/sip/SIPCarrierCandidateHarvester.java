package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;

public class SIPCarrierCandidateHarvester implements DialCandidateHarvester {

	@Override
	public List<DialCandidate> getCandidatesForNumber(Context context,
			String dialedNumber, String e164Number) {
		
		List<DialCandidate> candidates = new Vector<DialCandidate>();
		
		LumicallDataSource ds = new LumicallDataSource(context);
		ds.open();
		
		List<SIPIdentity> sipIdentities = ds.getSIPIdentities();
		
		for(SIPIdentity s : sipIdentities) {
			String prefix = s.getCarrierIntlPrefix();
			String uri = s.getUri();
			String domain = uri.substring(uri.indexOf('@') + 1);
			if(prefix != null && prefix.length() > 0) {
				String sipAddress = prefix + e164Number.substring(1) // Strip off the leading +
						+ "@" + domain;
				candidates.add(new DialCandidate("sip", sipAddress, "", "VoIP Carrier"));
			}
		}
		
		ds.close();
		
		return candidates;
	}

}
