/**
 * Copyright (C) 2012 Ready Technology (UK) Limited
 * 
 * Part of this file borrowed from Mobicents
 * 
 * Licensed under the GPL v3
 */


package org.sipdroid.media;

public abstract class RtpClock {
	
	// Packet size in ms
	int sampleRate;
    
    protected long now;
    private boolean isSynchronized;
    
    public RtpClock() {
    }
    
    public int getSampleRate() {
        return sampleRate;
    }
    
    public void setSampleRate(int SampleRate) {
        this.sampleRate = SampleRate;
    }
    
    public void synchronize(long initial) {
        now = initial;
        this.isSynchronized = true;
    }
    
    public boolean isSynchronized() {
        return this.isSynchronized();
    }
    
    protected long now() {
        return now;
    }
    
    public void reset() {
        now = 0;
        this.isSynchronized = false;
        this.sampleRate = -1;
    }
    
    /**
     * Returns the time in milliseconds
     * 
     * @param timestamp the rtp timestamp
     * @return the time in milliseconds
     */
    public abstract long getTime(long timestamp);
    
    /**
     * Calculates RTP timestamp
     * 
     * @param time the time in milliseconds
     * @return rtp timestamp.
     */
    public abstract long getTimestamp(long time);
}
