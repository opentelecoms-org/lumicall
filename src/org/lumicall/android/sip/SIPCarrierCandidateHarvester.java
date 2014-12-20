package org.lumicall.android.sip;

import java.util.List;

import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;

import android.content.Context;

public class SIPCarrierCandidateHarvester extends DialCandidateHarvester {
	
	public final static String SOURCE_INFO = "VoIP Carrier";
	
	Context context;
	
	public SIPCarrierCandidateHarvester(Context context) {
		this.context = context;
	}

	@Override
	public void getCandidatesForNumber(String dialedNumber,
			String e164Number) {
		
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
				onDialCandidateFound(new DialCandidate("sip", sipAddress, "", SOURCE_INFO, s));
			}
		}
		
		ds.close();
		
		onHarvestCompletion();
	}

}
