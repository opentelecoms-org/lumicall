package org.lumicall.android.sip;

import java.util.Collection;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

import org.xbill.DNS.ExtendedResolver;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.Resolver;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

import android.content.Context;
import android.util.Log;

public class HarvestDirector {
	
	private static final String TAG = "HarvestDirector";
	List<DialCandidateHarvester> harvesters;

	public HarvestDirector() {
		
		harvesters = new Vector<DialCandidateHarvester>();
		
		harvesters.add(new ENUMCandidateHarvester());
		harvesters.add(new EmailCandidateHarvester());
		harvesters.add(new SIPCarrierCandidateHarvester());
		harvesters.add(new GSMCandidateHarvester());
		
	}
	
	public List<DialCandidate> getCandidates(Context context, String number, String e164Number) {
		Vector<DialCandidate> candidates = new Vector<DialCandidate>();
		
		CyclicBarrier b = new CyclicBarrier(harvesters.size() + 1);
		Vector<HarvestThread> v = new Vector<HarvestThread>();
		
		for(DialCandidateHarvester h : harvesters) {
			HarvestThread t = new HarvestThread(b, h, context, number, e164Number);
			v.add(t);
			t.start();
		}
		
		// Wait for all lookups to finish
		try {
			b.await();
		} catch (InterruptedException e) {
			Log.e(TAG, "InterruptedException", e);
		} catch (BrokenBarrierException e) {
			Log.e(TAG, "BrokenBarrierException", e);
		}

		// Process the results
		// Notice that we process them in the order of the suffixes
		// Earlier results should have priority over later results
		for(HarvestThread t : v) {
			candidates.addAll(t.getDialCandidates());
		}

		return candidates;
	}
	
	private static class HarvestThread extends Thread {
		
		CyclicBarrier barrier;
		DialCandidateHarvester h;
		Context context;
		String number;
		String e164Number;
		List<DialCandidate> candidates;
		
		HarvestThread(CyclicBarrier barrier, DialCandidateHarvester h, Context context, String number, String e164Number) {
			this.barrier = barrier;
			this.h = h;
			this.context = context;
			this.number = number;
			this.e164Number = e164Number;
			candidates = new Vector<DialCandidate>();
		}
		
		public Collection<? extends DialCandidate> getDialCandidates() {
			return candidates;
		}

		public void run() {
			try {
				try {
					
					candidates.addAll(h.getCandidatesForNumber(context, number, e164Number));
				
				} catch (Exception ex) {
					Log.w(TAG, "Exception during harvest: ", ex);
				}
				barrier.await();
			} catch (InterruptedException ex) {
				ex.printStackTrace();
			} catch (BrokenBarrierException ex) {
				ex.printStackTrace();
			}
		}
	}

	
	
}
