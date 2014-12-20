package org.lumicall.android.sip;

import java.util.ArrayList;

public abstract class ThreadedDialCandidateHarvester extends DialCandidateHarvester implements Runnable {

	ArrayList<Thread> threads = new ArrayList<Thread>();

	@Override
	public void getCandidatesForNumber(String dialedNumber,
			String e164Number) {

		createThreads(dialedNumber, e164Number);
		
		new Thread(this).start();
	}

	@Override
	public void run() {
		for(Thread t : threads) {
			while(t.isAlive()) {
				try {
					t.join();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		onHarvestCompletion();	
	}
	
	protected void addThread(Thread t) {
		threads.add(t);
	}

	protected abstract void createThreads(String dialedNumber,
			String e164Number);
}
