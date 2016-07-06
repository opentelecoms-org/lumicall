/*
 * Copyright 2011-2016 Daniel Pocock
 * 
 * Based on code from ENUMDroid (Copyright 2009 Nominet UK)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lumicall.android.sip;

import java.io.IOException;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

import uk.nominet.DDDS.ENUM;
import uk.nominet.DDDS.Rule;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ENUMClient {

	private static final Logger logger = LoggerFactory.getLogger(ENUMClient.class);
	
	List<String> suffixes;

	public ENUMClient(List<String> suffixes) {
		this.suffixes = suffixes;
	}

	protected void parseRule(Rule rule, List<ENUMResult> results)
	{
		// split service field on '+' token
		String[] services = rule.getService().toLowerCase().split("\\+");
		
		// check that resulting fields are valid
		if (services.length != 2) return;	// not x+y
		if (!services[0].equals("e2u")) return; // not E2U+...
		
		// Only parse SIP-compatible records, others are silently
		// ignored
		for(String s : services) {
			if(s.equals("sip")) {
				String result = rule.evaluate();
				if (result != null) {
					logger.debug("sip" + " -> " + result);
					results.add(new ENUMResult("sip", result));
				}
			} 
		}
	}
	
	private static class LookupThread extends Thread {
		Rule[] rules;
		CyclicBarrier barrier;
		String suffix;
		String number;
		private ENUM mENUM = null;

		LookupThread(CyclicBarrier barrier, String number, String suffix) {
			this.barrier = barrier;
			rules = new Rule[] {};
			this.suffix = suffix;
			this.number = number;
		}

		public Rule[] getRules() {
			return rules;
		}

		public void run() {
			try {
				logger.debug("looking up " + number + " in " + suffix);
				mENUM = new ENUM(suffix);
				rules = mENUM.lookup(number);

				barrier.await();
			} catch (InterruptedException ex) {
				ex.printStackTrace();
			} catch (BrokenBarrierException ex) {
				ex.printStackTrace();
			}
		}
	}
	 
	public List<ENUMResult> query(String number) {
		// Setup variables
		CyclicBarrier b = new CyclicBarrier(suffixes.size() + 1);
		Vector<LookupThread> v = new Vector<LookupThread>();
		
		// Run the lookups in parallel
		for(String suffix : suffixes) {
			LookupThread t = new LookupThread(b, number, suffix);
			v.add(t);
			t.start();
		}
		
		// Wait for all lookups to finish
		try {
			b.await();
		} catch (InterruptedException e) {
			logger.error("InterruptedException", e);
		} catch (BrokenBarrierException e) {
			logger.error("BrokenBarrierException", e);
		}
		
		// Process the results
		// Notice that we process them in the order of the suffixes
		// Earlier results should have priority over later results
		List<ENUMResult> results = new Vector<ENUMResult>();
		for(LookupThread t : v) {
			for (Rule rule : t.getRules()) {
				parseRule(rule, results);
			}
		}
		return results;
	}

}
