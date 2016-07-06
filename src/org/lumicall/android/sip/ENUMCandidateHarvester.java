package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ENUMCandidateHarvester extends DialCandidateHarvester implements Runnable {

	private static final String ENUM_SUFFIX = "e164.arpa";
	
	private static final Logger logger = LoggerFactory.getLogger(ENUMCandidateHarvester.class);
	private String e164Number;
	
	public ENUMCandidateHarvester() {
	}

	protected boolean isOnline() {
		return true;
	}

	protected List<String> getSuffixes() {
		List<String> results = new Vector<String>();
		results.add(ENUM_SUFFIX);
		return results;
	}

	@Override
	public void getCandidatesForNumber(String number, String e164Number) {
		
		boolean runHarvest = true;
		
		if(!isOnline()) {
			logger.warn("ENUM not online");
			runHarvest = false;
		}
		
		if(e164Number == null || !e164Number.startsWith("+")) {
			logger.warn("can't handle non-E.164 numbers");
			runHarvest = false;
		}
		
		if(runHarvest) {
			this.e164Number = e164Number;
			Thread t = new Thread(this);
			t.start();
		} else {
			onHarvestCompletion();
		}
	}
	
	/**
	 * This is not ideal, it does all ENUM lookups in one thread and then
	 * returns all the results in one go.  It would be better to
	 * return each result as they arrive from DNS.
	 */
	@Override
	public void run() {
		ENUMClient client = new ENUMClient(getSuffixes());
		List<ENUMResult> results = client.query(e164Number);
		
		/* none found - tell the user then dial the original number */
		if (results == null || results.size() <= 0) {
			logger.debug("no ENUM result found");
			onHarvestCompletion();
			return;
		}

		int count = 0;
		for(ENUMResult result : results) {
			String destination = result.getResult();
			// Prevent prefix sip: from appearing twice (we add it again later)
			if(destination.startsWith("sip:"))
				destination = destination.substring(4);
			onDialCandidateFound(new DialCandidate("sip", destination, "", "ENUM"));
			count++;
		}
		
		logger.debug("ENUM results found, count = " + count);
		
		onHarvestCompletion();
	}

}
