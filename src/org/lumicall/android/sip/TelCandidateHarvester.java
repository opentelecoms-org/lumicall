package org.lumicall.android.sip;

public class TelCandidateHarvester extends DialCandidateHarvester {

	@Override
	public void getCandidatesForNumber(String dialedNumber,
			String e164Number) {
		onDialCandidateFound(new DialCandidate("tel", dialedNumber, "", "PSTN"));
		onHarvestCompletion();
	}
}
