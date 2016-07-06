package org.lumicall.android.sip;

import java.util.Collection;
import java.util.concurrent.ConcurrentLinkedQueue;

public class HarvestDirector extends DialCandidateHarvester implements DialCandidateListener {
	
	Collection<DialCandidateHarvester> harvesters;

	public HarvestDirector() {
		
		harvesters = new ConcurrentLinkedQueue<DialCandidateHarvester>();
		
		// The dnsjava stuff gives `bad address family' exceptions on the emulator
		// if IPv6 is preferred (for Android 2.2)
		System.setProperty("java.net.preferIPv6Addresses", "false");  // FIXME IPv6 support
	}

	public void addHarvester(DialCandidateHarvester h) {
		harvesters.add(h);
	}
	
	public void getCandidatesForNumber(String number, String e164Number) {
		for(DialCandidateHarvester h : harvesters) {
			h.addListener(this);
			h.getCandidatesForNumber(number, e164Number);
		}
	}
	
	@Override
	public void onDialCandidate(DialCandidateHarvester h, DialCandidate dc) {
		onDialCandidateFound(dc);
	}

	@Override
	public void onHarvestCompletion(DialCandidateHarvester h, final int resultCount) {
		h.removeListener(this);
		if(harvesters.contains(h)) {
			harvesters.remove(h);
		}
		if(harvesters.size() == 0) {
			this.onHarvestCompletion();
		}
	}
}
