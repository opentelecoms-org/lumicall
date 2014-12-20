package org.lumicall.android.sip;

import java.util.Collection;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public abstract class DialCandidateHarvester {
	
	Collection<DialCandidateListener> listeners =
			new ConcurrentLinkedQueue<DialCandidateListener>();
	
	AtomicInteger resultCount = new AtomicInteger(0);
	
	public void addListener(DialCandidateListener l) {
		listeners.add(l);
	}
	
	public void removeListener(DialCandidateListener l) {
		listeners.remove(l);
	}

	protected void onDialCandidateFound(DialCandidate dialCandidate) {
		resultCount.incrementAndGet();
		for(DialCandidateListener l : listeners) {
			l.onDialCandidate(this, dialCandidate);
		}
	}

	protected void onHarvestCompletion() {
		for(DialCandidateListener l : listeners) {
			l.onHarvestCompletion(this, resultCount.get());
		}
	}
	
	public abstract void getCandidatesForNumber(String dialedNumber, String e164Number);
}
