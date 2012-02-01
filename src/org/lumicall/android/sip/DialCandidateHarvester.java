package org.lumicall.android.sip;

import java.util.List;

import android.content.Context;

public interface DialCandidateHarvester {

	public List<DialCandidate> getCandidatesForNumber(Context context, String dialedNumber, String e164Number);
	
}
