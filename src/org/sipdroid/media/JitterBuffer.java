/**
 * Copyright (C) 2012 Ready Technology (UK) Limited
 * 
 * Part of this file borrowed from Mobicents
 * 
 * Licensed under the GPL v3
 */

package org.sipdroid.media;


import java.io.Serializable;
import java.util.logging.Logger;

import org.sipdroid.net.RtpPacket;

/**
 * Implements jitter buffer.
 * 
 * A jitter buffer temporarily stores arriving packets in order to minimize
 * delay variations. If packets arrive too late then they are discarded. A
 * jitter buffer may be mis-configured and be either too large or too small.
 * 
 * If a jitter buffer is too small then an excessive number of packets may be
 * discarded, which can lead to call quality degradation. If a jitter buffer is
 * too large then the additional delay can lead to conversational difficulty.
 * 
 * A typical jitter buffer configuration is 30mS to 50mS in size. In the case of
 * an adaptive jitter buffer then the maximum size may be set to 100-200mS. Note
 * that if the jitter buffer size exceeds 100mS then the additional delay
 * introduced can lead to conversational difficulty.
 * 
 * @author Oleg Kulikov
 * @author amit bhayani
 * @author Daniel Pocock
 */
public class JitterBuffer implements Serializable {

    private int period;
    private int jitter;
    private int jitterSamples;
    private BufferConcurrentLinkedQueue<RtpPacket> queue = new BufferConcurrentLinkedQueue<RtpPacket>();
    private volatile boolean ready = false;
    
    private long duration;
    private volatile long timestamp;
    //private Format format;
    private int sampleRate;
    private RtpClock clock;
    private Logger logger = Logger.getLogger(getClass().getName());
    private long delta;
    
    Object lock = new Object();
    
    /**
     * Creates new instance of jitter.
     * 
     * @param fmt
     *            the format of the received media
     * @param jitter
     *            the size of the jitter in milliseconds.
     */
    public JitterBuffer(int jitter, int period) {
        this.period = period;
        setJitter(jitter);
    }

    public void setClock(RtpClock clock) {
        this.clock = clock;
        if (sampleRate > 0) {
            clock.setSampleRate(sampleRate);
            setJitter(jitter);
        }
    }

    public void setSampleRate(int sampleRate) {
        this.sampleRate = sampleRate;
        if (clock != null) {
            clock.setSampleRate(sampleRate);
        }
    }
    
    private void setJitter(int jitter) {
    	this.jitter = jitter;
    	if(clock != null)
    		this.jitterSamples = (int)clock.getTimestamp(jitter); 
    }

    public int getJitter() {
        return jitter;
    }

    public void setPeriod(int period) {
        this.period = period;
    }

    public void write(RtpPacket rtpPacket) {
        long t = clock.getTime(rtpPacket.getTimestamp());
        
        synchronized(lock) {

        //when first packet arrive and timestamp already known
        //we have to determine difference between rtp stream timestamps and local time
        if (delta == 0 && timestamp > 0) {
            delta = t - timestamp;
            timestamp += delta;
        }
        
        //if buffer's ready flag equals true then it means that reading 
        //starting and we should compare timestamp of arrived packet with time of
        //last reading.
        logger.info("RX packet: rx ts = " + t + ", local ts = " + timestamp + ", diff = " + (t - timestamp));
        if (ready && t > timestamp + jitterSamples) {
            //silentrly discard otstanding packet
            logger.warning("Packet " + rtpPacket + " is discarded by jitter buffer");
            return;
        }
        }
    
        //if RTP packet is not outstanding or reading not started yet (ready == false)
        //queue packet.
        queue.offer(rtpPacket);
        
        //allow read when buffer is full;
        duration += period;
        if (!ready && duration > (period + jitter)) {
            ready = true;
        }
    }

    public void reset() {
        queue.clear();
        duration = 0;
        clock.reset();
        delta = 0;
    }

    /**
     * 
     * @return
     */
    public RtpPacket read(long timestamp) {
        //discard buffer is buffer is not full yet
        if (!ready) {
            return null;
        }

        synchronized(lock) {
        //remember timestamp
        this.timestamp = timestamp + delta;
        }
        
        //if packet queue is empty (but was full) we have to returns
        //silence
        if (queue.isEmpty()) {
            return null;
        }

        //fill media buffer
        return queue.poll();
    }
}
