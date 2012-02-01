package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;

import android.content.Context;

public class VoIPCandidateHarvester implements DialCandidateHarvester {

	@Override
	public List<DialCandidate> getCandidatesForNumber(Context context,
			String dialedNumber, String e164Number) {
		
		List<DialCandidate> candidates = new Vector<DialCandidate>();
		
		// TODO Implement: iterate over all SIPIdentities
		
		return candidates;
	}

}
