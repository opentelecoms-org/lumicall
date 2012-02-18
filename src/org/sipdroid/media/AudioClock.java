/**
 * Copyright (C) 2012 Ready Technology (UK) Limited
 * 
 * Licensed under the GPL v3
 */


package org.sipdroid.media;

public class AudioClock extends RtpClock {


	private double packetPeriod;

	@Override
	public void setSampleRate(int sampleRate) {
		super.setSampleRate(sampleRate);
		this.packetPeriod = 1000/sampleRate;
	}

	@Override
	public long getTime(long timestamp) {
		return (long) (packetPeriod * timestamp);
	}

	@Override
	public long getTimestamp(long time) {
		return (long) (time/packetPeriod);
	}

}


