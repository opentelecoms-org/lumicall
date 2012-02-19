/**
 * Copyright (C) 2012 Ready Technology (UK) Limited
 * 
 * Licensed under the GPL v3
 */


package org.sipdroid.media;

public class AudioClock extends RtpClock {


	private double samplesPerMillisecond;

	@Override
	public void setSampleRate(int sampleRate) {
		super.setSampleRate(sampleRate);
		this.samplesPerMillisecond = sampleRate / 1000;
	}

	@Override
	public long getTime(long timestamp) {
		return (long) (samplesPerMillisecond * timestamp);
	}

	@Override
	public long getTimestamp(long time) {
		return (long) (time/samplesPerMillisecond);
	}

}


