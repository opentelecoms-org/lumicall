package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;

import android.content.Context;

public class GSMCandidateHarvester implements DialCandidateHarvester {

	@Override
	public List<DialCandidate> getCandidatesForNumber(Context context,
			String dialedNumber, String e164Number) {
		
		List<DialCandidate> candidates = new Vector<DialCandidate>();
		
		candidates.add(
				new DialCandidate("tel", dialedNumber, "", "GSM"));
				
		return candidates;		
		
	}

}
