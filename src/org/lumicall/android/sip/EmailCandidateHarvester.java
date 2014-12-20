package org.lumicall.android.sip;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import org.xbill.DNS.ExtendedResolver;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.NAPTRRecord;
import org.xbill.DNS.Record;
import org.xbill.DNS.Resolver;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

import android.util.Log;

public abstract class EmailCandidateHarvester extends ThreadedDialCandidateHarvester {
	
	private String[] protocols = { "_sips._tcp" };
	private String[] services = { "SIP+D2T" };
	
	private Set<String> svcSet = new HashSet<String>();
	
	public EmailCandidateHarvester() {
		//System.setProperty("dns.server", ENUMProviderForSIP.ROOT_SERVERS);
		for(String svc : services) {
			svcSet.add(svc.toLowerCase());
		}
	}
	
	final static String TAG = "EmailHarvester";
	
	private class LookupThread extends Thread {
		
		private static final int DNS_TIMEOUT = 2;
		DialCandidate sipCandidate;
		String key;
		int type;
		
		LookupThread(DialCandidate sipCandidate, String key, int type) {
			this.sipCandidate = sipCandidate;
			this.key = key;
			this.type = type;
		}
		
		public void run() {
			Log.v(TAG, "looking up " + key);

			try {
				Resolver resolver = new ExtendedResolver();
				resolver.setTimeout(DNS_TIMEOUT);
				Lookup lookup = new Lookup(key, type);
				lookup.setResolver(resolver);
				Record[] records = lookup.run();

				if (records != null) {
					Log.v(TAG, "found " + records.length + " record(s)");
					for (int i = 0; i < records.length; ++i) {
						// type check necessary in case of other RRtypes in the
						// Answer
						// Section
						if (records[i] instanceof SRVRecord) {
							// Found an SRV record, candidate is good
							Log.v(TAG, "found matching SRV record for: " + key);
							onDialCandidateFound(sipCandidate);
							return;
						} else if(records[i] instanceof NAPTRRecord) {
							NAPTRRecord r = (NAPTRRecord)records[i];
							if(svcSet.contains(r.getService().toLowerCase())) {
								Log.v(TAG, "found matching SRV record for: " + key);
								onDialCandidateFound(sipCandidate);
								return;
							}
						}
					}
				} else {
					Log.v(TAG, "records == null, result = " + lookup.getErrorString());
				}
			} catch (Exception ex) {
				Log.w(TAG, "Exception during DNS lookup: ", ex);
			}
		}
	}

	@Override
	public void createThreads(String number, String e164Number) {

		Collection<DialCandidate> candidates = getEmailAddressesForNumber(e164Number);
		
		if(candidates == null || candidates.size() == 0) {
			onHarvestCompletion();
			return;
		}
		
		for(DialCandidate c : candidates) {
			// For each candidate, check if SRV record exists
			// We should check NAPTR too
			for(String protocol : protocols) {
				String key = protocol + "." + c.getDomain() + ".";  // trailing dot required
				LookupThread t = new LookupThread(c, key, Type.SRV);
				t.start();
			}
			LookupThread t = new LookupThread(c, c.getDomain(), Type.NAPTR);
			t.start();
			addThread(t);
		}
	}
	
	protected abstract Collection<DialCandidate> getEmailAddressesForNumber(String number);	
	
}
