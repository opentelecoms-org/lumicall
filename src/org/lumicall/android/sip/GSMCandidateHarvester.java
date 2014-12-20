package org.lumicall.android.sip;

public class GSMCandidateHarvester extends DialCandidateHarvester {

	@Override
	public void getCandidatesForNumber(String dialedNumber,
			String e164Number) {
		onDialCandidateFound(new DialCandidate("tel", dialedNumber, "", "GSM"));
		onHarvestCompletion();
	}
}
