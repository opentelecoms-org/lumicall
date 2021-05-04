/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
 * Copyright (C) 2005 Luca Veltri - University of Parma - Italy
 * 
 * This file is part of Sipdroid (http://www.sipdroid.org)
 * 
 * Sipdroid is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this source code; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package org.sipdroid.media;

import java.io.IOException;
import java.io.InputStream;
import java.math.BigInteger;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

import zorg.SRTP;
import zorg.ZRTP;
import org.opentelecoms.util.CRC32C;
import org.sipdroid.net.RtpPacket;
import org.sipdroid.net.RtpSocket;
import org.sipdroid.sipua.UserAgent;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;
import org.sipdroid.sipua.ui.Sipdroid;
import org.sipdroid.codecs.Codecs;
import org.sipdroid.codecs.G711;


import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.SystemClock;
import android.preference.PreferenceManager;

/**
 * RtpStreamSender is a generic stream sender. It takes an InputStream and sends
 * it through RTP.
 */
public class RtpStreamSender extends Thread {
	
	private static final Logger logger =
	        Logger.getLogger(RtpStreamSender.class.getName());

	public static final int FIRST_SEQ_NUM = 1;

	private static final int BUF_SIZE = 4096;
	
	/** Whether working in debug mode. */
	public static boolean DEBUG = true;

	/** The RtpSocket */
	RtpSocket rtp_socket = null;
	
	byte[] buffer;
	RtpPacket rtp_packet;

	/** Payload type */
	Codecs.Map p_type;

	/** Number of frame per second */
	int frame_rate;

	/** Number of bytes per frame */
	int frame_size;

	/**
	 * Whether it works synchronously with a local clock, or it it acts as slave
	 * of the InputStream
	 */
	boolean do_sync = true;

	/**
	 * Synchronization correction value, in milliseconds. It accellarates the
	 * sending rate respect to the nominal value, in order to compensate program
	 * latencies.
	 */
	int sync_adj = 0;

	/** Whether it is running */
	boolean running = false;
	boolean muted = false;
	
	//DTMF change
	String dtmf = "";
	int dtmf_payload_type = 101;
	
	private static HashMap<Character, Byte> rtpEventMap = new HashMap<Character,Byte>(){{
		put('0',(byte)0);
		put('1',(byte)1);
		put('2',(byte)2);
		put('3',(byte)3);
		put('4',(byte)4);
		put('5',(byte)5);
		put('6',(byte)6);
		put('7',(byte)7);
		put('8',(byte)8);
		put('9',(byte)9);
		put('*',(byte)10);
		put('#',(byte)11);
		put('A',(byte)12);
		put('B',(byte)13);
		put('C',(byte)14);
		put('D',(byte)15);
	}};
	//DTMF change 
	
	CallRecorder call_recorder = null;
	
	volatile SRTP srtp;
	volatile ZRTP zrtp;
	volatile ZRTPHelper zrtpHelper = null;
	
	/**
	 * Constructs a RtpStreamSender.
	 * 
	 * @param input_stream
	 *            the stream to be sent
	 * @param do_sync
	 *            whether time synchronization must be performed by the
	 *            RtpStreamSender, or it is performed by the InputStream (e.g.
	 *            the system audio input)
	 * @param payload_type
	 *            the payload type
	 * @param frame_rate
	 *            the frame rate, i.e. the number of frames that should be sent
	 *            per second; it is used to calculate the nominal packet time
	 *            and,in case of do_sync==true, the next departure time
	 * @param frame_size
	 *            the size of the payload
	 * @param src_socket
	 *            the socket used to send the RTP packet
	 * @param dest_addr
	 *            the destination address
	 * @param dest_port
	 *            the destination port
	 * @param srtp 
	 * @param zrtp 
	 */
	public RtpStreamSender(boolean do_sync, Codecs.Map payload_type,
			       long frame_rate, int frame_size,
			       DatagramSocket src_socket, String dest_addr,
			       int dest_port, CallRecorder rec, SRTP srtp, ZRTP zrtp) {
		init(do_sync, payload_type, frame_rate, frame_size,
				src_socket, dest_addr, dest_port);
		call_recorder = rec;
		this.srtp = srtp;
		this.zrtp = zrtp;
		if(zrtp != null) {
			zrtpHelper = new ZRTPHelper();
			zrtp.setRtpStack(zrtpHelper);
		}
	}

	/** Inits the RtpStreamSender */
	private void init(boolean do_sync, Codecs.Map payload_type,
			  long frame_rate, int frame_size,
			  DatagramSocket src_socket, String dest_addr,
			  int dest_port) {
		this.p_type = payload_type;
		this.frame_rate = (int)frame_rate;
		buffer = new byte[BUF_SIZE];
		rtp_packet = new RtpPacket(buffer, buffer.length);
		//boolean isPBXes = PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(Settings.PREF_SERVER, "").equals(Settings.DEFAULT_SERVER);
		boolean isPBXes = false;
		if (isPBXes)
			switch (payload_type.codec.number()) {
			case 0:
			case 8:
				this.frame_size = 1024;
				break;
			case 9:
				this.frame_size = 960;
				break;
			case 18:
                                this.frame_size = 80;
			default:
				this.frame_size = frame_size;
				break;
			}
		else
			this.frame_size = frame_size;
		this.do_sync = do_sync;
		try {
			InetAddress dest = InetAddress.getByName(dest_addr);
			logger.info("dest_addr: " + dest_addr + "(" + dest + ") port: " + dest_port);
			rtp_socket = new RtpSocket(src_socket, dest, dest_port);
		} catch (Exception e) {
			if (!Sipdroid.release) e.printStackTrace();
		}
		setForceTX(false);
	}

	/** Sets the synchronization adjustment time (in milliseconds). */
	public void setSyncAdj(int millisecs) {
		sync_adj = millisecs;
	}

	/** Whether is running */
	public boolean isRunning() {
		return running;
	}
	
	public boolean mute() {
		return muted = !muted;
	}

	public static int delay = 0;
	public static boolean changed;
	
	/** Stops running */
	public void halt() {
		running = false;
	}

	Random random;
	double smin = 200,s;
	int nearend;
	
	void calc(short[] lin,int off,int len) {
		int i,j;
		double sm = 30000,r;
		
		for (i = 0; i < len; i += 5) {
			j = lin[i+off];
			s = 0.03*Math.abs(j) + 0.97*s;
			if (s < sm) sm = s;
			if (s > smin) nearend = 3000*mu/5;
			else if (nearend > 0) nearend--;
		}
		r = (double)len/(100000*mu);
		if (sm > 2*smin || sm < smin/2)
			smin = sm*r + smin*(1-r);
	}

	void calc1(short[] lin,int off,int len) {
		int i,j;
		
		for (i = 0; i < len; i++) {
			j = lin[i+off];
			lin[i+off] = (short)(j>>2);
		}
	}

	void calc2(short[] lin,int off,int len) {
		int i,j;
		
		for (i = 0; i < len; i++) {
			j = lin[i+off];
			lin[i+off] = (short)(j>>1);
		}
	}

	void calc10(short[] lin,int off,int len) {
		int i,j;
		
		for (i = 0; i < len; i++) {
			j = lin[i+off];
			if (j > 16350)
				lin[i+off] = 16350<<1;
			else if (j < -16350)
				lin[i+off] = -16350<<1;
			else
				lin[i+off] = (short)(j<<1);
		}
	}

	void noise(short[] lin,int off,int len,double power) {
		int i,r = (int)(power*2);
		short ran;

		if (r == 0) r = 1;
		for (i = 0; i < len; i += 4) {
			ran = (short)(random.nextInt(r*2)-r);
			lin[i+off] = ran;
			lin[i+off+1] = ran;
			lin[i+off+2] = ran;
			lin[i+off+3] = ran;
		}
	}
	
	protected void sendPacket(RtpPacket p) throws IOException {
		
		boolean encrypted = false;
		if(srtp != null) {
			if(srtp.protect(p) == null)
				throw new RuntimeException("something went wrong in SRTP.protect()");
			encrypted = true;
		}
		if(zrtp != null) {
			if(encrypted || !zrtp.isProgressing())
				rtp_socket.send(p);
			if(zrtpHelper != null)
				zrtpHelper.sendPackets();
		} else
			rtp_socket.send(p);
	}
	
	boolean forceTX = false;
	public void setForceTX(boolean forceTX) {
		this.forceTX = forceTX;
	}
	
	public static int m;
	int mu;
	
	int seqn = FIRST_SEQ_NUM;
	
	/** Runs it in a new Thread. */
	public void run() {
		WifiManager wm = (WifiManager) Receiver.mContext.getSystemService(Context.WIFI_SERVICE);
		long lastscan = 0,lastsent = 0;

		if (rtp_socket == null) {
			logger.warning("rtp_socket == null, RtpStreamSender.run aborting");
			return;
		}
		
		long time = 0;
		double p = 0;
		// boolean improve = PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getBoolean(Settings.PREF_IMPROVE, Settings.DEFAULT_IMPROVE);
		boolean improve = false;  // FIXME - what is the impact of this option?
		boolean selectWifi = PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getBoolean(org.sipdroid.sipua.ui.Settings.PREF_SELECTWIFI, org.sipdroid.sipua.ui.Settings.DEFAULT_SELECTWIFI);
		int micgain = 0;
		long last_tx_time = 0;
		long next_tx_delay;
		long now;
		running = true;
		m = 1;
		int dtframesize = 4;
		
		android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
		mu = p_type.codec.samp_rate()/8000;
		int min = AudioRecord.getMinBufferSize(p_type.codec.samp_rate(), 
				AudioFormat.CHANNEL_CONFIGURATION_MONO, 
				AudioFormat.ENCODING_PCM_16BIT);
		if (min == 640) {
			if (frame_size == 960) frame_size = 320;
			if (frame_size == 1024) frame_size = 160;
			min = 4096*3/2;
		} else if (min < 4096) {
			if (min <= 2048 && frame_size == 1024) frame_size /= 2;
			min = 4096*3/2;
		} else if (min == 4096) {
			min *= 3/2;
			if (frame_size == 960) frame_size = 320;
		} else {
			if (frame_size == 960) frame_size = 320;
			if (frame_size == 1024) frame_size *= 2;
		}
		frame_rate = p_type.codec.samp_rate()/frame_size;
		long frame_period = 1000 / frame_rate;
		frame_rate *= 1.5;
		//byte[] buffer = new byte[2 * (frame_size + 12 + 4)];  // FIXME - good size for ZRTP packets?
		rtp_packet.setPayloadType(p_type.number);
		if (DEBUG)
			println("Reading blocks of " + buffer.length + " bytes");
		
		println("Sample rate  = " + p_type.codec.samp_rate());
		println("Buffer size = " + min);

		AudioRecord record = null;
		
		short[] lin = new short[frame_size*(frame_rate+1)];
		int num,ring = 0,pos;
		random = new Random();
		InputStream alerting = null;
		try {
			alerting = Receiver.mContext.getAssets().open("alerting");
		} catch (IOException e2) {
			if (!Sipdroid.release) e2.printStackTrace();
		}
		
		long maximumIntervalBetweenTransmissions = frame_period;
		
		p_type.codec.init();
		
		if(zrtp != null)
			zrtp.startSession();
		
		while (running) {
			 if (changed || record == null) {
				if (record != null) {
					record.stop();
					record.release();
					if (RtpStreamReceiver.samsung) {
						AudioManager am = (AudioManager) Receiver.mContext.getSystemService(Context.AUDIO_SERVICE);
						am.setMode(AudioManager.MODE_IN_CALL);
						am.setMode(AudioManager.MODE_NORMAL);
					}
				}
				changed = false;
				record = new AudioRecord(MediaRecorder.AudioSource.MIC, p_type.codec.samp_rate(), AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, 
							min);
				if (record.getState() != AudioRecord.STATE_INITIALIZED) {
					Receiver.engine(Receiver.mContext).rejectcall();
					record = null;
					break;
				}
				record.startRecording();
				micgain = (int)(Settings.getMicGain()*10);
			 }
			 if (muted || Receiver.call_state == UserAgent.UA_STATE_HOLD) {
				if (Receiver.call_state == UserAgent.UA_STATE_HOLD)
					RtpStreamReceiver.restoreMode();
				record.stop();
				while (running && (muted || Receiver.call_state == UserAgent.UA_STATE_HOLD)) {
					try {
						logger.finer("RtpStreamSender: sleeping 1s while on-hold/muted");
						sleep(1000);
					} catch (InterruptedException e1) {
					}
				}
				record.startRecording();
			 }
			 //DTMF change start
			 if (dtmf.length() != 0) {
				 logger.fine("must send DTMF code '" + dtmf.charAt(0) + "'");
				 int dtmf_buf_size = dtframesize + 12 + (srtp != null ? 10 : 0);
	 			 byte[] dtmfbuf = new byte[dtmf_buf_size];
				 RtpPacket dt_packet = new RtpPacket(dtmfbuf, 0);
				 dt_packet.setPayloadType(dtmf_payload_type);
 				 dt_packet.setPayloadLength(dtframesize);
				 dt_packet.setSscr(rtp_packet.getSscr());
				 long dttime = time;
				 int duration;
				 
	 			 for (int i = 0; i < 6; i++) { 
					 logger.finer("sending DTMF start");
 	 				 time += 160;
 	 				 duration = (int)(time - dttime);
	 				 dt_packet.setSequenceNumber(seqn++);
	 				 dt_packet.setTimestamp(dttime);
	 				 dtmfbuf[12] = rtpEventMap.get(dtmf.charAt(0));
	 				 dtmfbuf[13] = (byte)0x0a;
	 				 dtmfbuf[14] = (byte)(duration >> 8);
	 				 dtmfbuf[15] = (byte)duration;
	 				 try {
						sendPacket(dt_packet);
						sleep(20);
	 				 } catch (Exception e1) {
						 logger.log(Level.SEVERE, "failed to send DTMF start packet", e1);
	 				 }
	 			 }
	 			 for (int i = 0; i < 3; i++) {
					 logger.finer("sending DTMF stop");
	 				 duration = (int)(time - dttime);
	 				 dt_packet.setSequenceNumber(seqn);
	 				 dt_packet.setTimestamp(dttime);
	 				 dtmfbuf[12] = rtpEventMap.get(dtmf.charAt(0));
	 				 dtmfbuf[13] = (byte)0x8a;
	 				 dtmfbuf[14] = (byte)(duration >> 8);
	 				 dtmfbuf[15] = (byte)duration;
	 				 try {
						sendPacket(dt_packet);
	 				 } catch (Exception e1) {
						 logger.log(Level.SEVERE, "failed to send DTMF stop packet", e1);
	 				 }	 			 
	 			 }
	 			 time += 160; seqn++;
				dtmf=dtmf.substring(1);
			 }
			 //DTMF change end

			 if (frame_size < 480) {
				 now = System.currentTimeMillis();
				 next_tx_delay = frame_period - (now - last_tx_time);
				 last_tx_time = now;
				 if (next_tx_delay > 0) {
					 try {
						 sleep(next_tx_delay);
					 } catch (InterruptedException e1) {
					 }
					 last_tx_time += next_tx_delay-sync_adj;
				 }
			 }
			 pos = (ring+delay*frame_rate*frame_size)%(frame_size*(frame_rate+1));
			 num = record.read(lin,pos,frame_size);
			 if (num <= 0) {
				if (num < 0)
					logger.warning("AndroidRecord.record.read returned error code: "
							+ num);
				else
					logger.warning("AndroidRecord.record.read returned nothing");
				continue;
			 }
			 if(num < frame_size)
				 logger.warning("AndroidRecord.record.read returned " + num + " samples but framesize = " + frame_size);
			 if (!p_type.codec.isValid())
				 continue;
			 
			 // Call recording: Save the frame to the CallRecorder.
			 if (call_recorder != null)
			 	call_recorder.writeOutgoing(lin, pos, num);

			 if (RtpStreamReceiver.speakermode == AudioManager.MODE_NORMAL) {
 				 calc(lin,pos,num);
 	 			 if (RtpStreamReceiver.nearend != 0 && RtpStreamReceiver.down_time == 0)
	 				 noise(lin,pos,num,p/2);
	 			 else if (nearend == 0)
	 				 p = 0.9*p + 0.1*s;
 			 } else switch (micgain) {
 			 case 1:
 				 calc1(lin,pos,num);
 				 break;
 			 case 2:
 				 calc2(lin,pos,num);
 				 break;
 			 case 10:
 				 calc10(lin,pos,num);
 				 break;
 			 }
			 if (!forceTX && Receiver.call_state != UserAgent.UA_STATE_INCALL &&
					 Receiver.call_state != UserAgent.UA_STATE_OUTGOING_CALL && alerting != null) {
				 try {
					if (alerting.available() < num/mu)
						alerting.reset();
					alerting.read(buffer,12,num/mu);
				 } catch (IOException e) {
					if (!Sipdroid.release) e.printStackTrace();
				 }
				 if (p_type.codec.number() != 8) {
					 G711.alaw2linear(buffer, lin, num, mu);
					 num = p_type.codec.encode(lin, 0, buffer, num);
				 }
			 } else {
				 int offset = ring%(frame_size*(frame_rate+1));
				 num = p_type.codec.encode(lin, offset, buffer, num);
			 }
			 
 
 			 ring += frame_size;
 			 rtp_packet.setSequenceNumber(seqn++);
 			 rtp_packet.setTimestamp(time);
 			 rtp_packet.setPayloadLength(num);
 			 now = SystemClock.elapsedRealtime();
 			 //if (RtpStreamReceiver.timeout == 0 || now-lastsent > maximumIntervalBetweenTransmissions)
 			 if(true)
	 			 try {
	 				 lastsent = now;
	 				 sendPacket(rtp_packet);
	 				 if (m == 2 && RtpStreamReceiver.timeout == 0)
	 					 rtp_socket.send(rtp_packet);  // can't use sendPacket here, or double encryption
	 			 } catch (Exception e) {
	 				 logger.log(Level.SEVERE, "Exception from rtp_socket.send(): ", e);
	 			 }
 			 else
 				 logger.info("not sending a packet now, RtpStreamReceiver.timeout = " +
 						 RtpStreamReceiver.timeout + " != 0");
 			 if (p_type.codec.number() == 9)
 				 time += frame_size/2;
 			 else
 				 time += frame_size;
 			 if (RtpStreamReceiver.good != 0 &&
 					 RtpStreamReceiver.loss2/RtpStreamReceiver.good > 0.01) {
 				 if (selectWifi && Receiver.on_wlan && now-lastscan > 10000) {
 					 wm.startScan(); // FIXME - should not do this from RTP thread
 					 lastscan = now;
 				 }
 				 if (improve && delay == 0 &&
 						 (p_type.codec.number() == 0 || p_type.codec.number() == 8 || p_type.codec.number() == 9))        	
 					 m = 2;
 				 else
 					 m = 1;
 			 } else
 				 m = 1;
		} // while running
		if (Build.VERSION.SDK_INT < 5)
			while (RtpStreamReceiver.getMode() == AudioManager.MODE_IN_CALL)
				try {
					logger.info("finished sending RTP, sleeping for 1000ms...");
					sleep(1000);
				} catch (InterruptedException e) {
				}
		if (record != null) {
			record.stop();
			record.release();
		}
		m = 0;
		
		p_type.codec.close();
		rtp_socket.close();
		rtp_socket = null;
		
		// Call recorder: stop recording outgoing.
		if (call_recorder != null)
		{
			call_recorder.stopOutgoing();
			call_recorder = null;
		}

		if (DEBUG)
			println("rtp sender terminated");
	}

	/** Debug output */
	private static void println(String str) {
		if (!Sipdroid.release) System.out.println("RtpStreamSender: " + str);
	}

	/** Set RTP payload type of outband DTMF packets. **/  
	public void setDTMFpayloadType(int payload_type){
		dtmf_payload_type = payload_type; 
		logger.info("using DTMF payload type " + dtmf_payload_type);
	}
	
	/** Send outband DTMF packets */
	public void sendDTMF(char c) {
		logger.fine("adding DTMF character '" + c + "' to queue");
		dtmf = dtmf+c; // will be set to 0 after sending tones
	}
	//DTMF change
	
	protected int getSscr() {
		return rtp_packet.getSscr();
	}
	
	public class ZRTPHelper implements zorg.platform.RtpStack {
		
		// We could be invoked from a different thread to the main sender,
		// so keep our own packet buffer
		//RtpPacket zrtp_packet;
		int seqNo;
		LinkedList<RtpPacket> pkts;
		
		public ZRTPHelper() {
			pkts = new LinkedList<RtpPacket>();
		}
		
		RtpPacket makePacket() {
			byte[] buf = new byte[BUF_SIZE];
			RtpPacket pkt = new RtpPacket(buf, buf.length);
			pkt.setTimestamp(ZRTP.ZRTP_MAGIC_COOKIE);
			pkt.setSscr(getSscr());
			return pkt;
		}

		@Override
		public void sendZrtpPacket(byte[] data) {
			try {
				RtpPacket zrtp_packet = makePacket();
				zrtp_packet.setPayload(data, data.length);
				if(seqNo > 0xffff)
					seqNo = 0;
				zrtp_packet.setSequenceNumber(seqNo++);
				appendCrc(zrtp_packet);
				sendPacket(zrtp_packet);
			} catch (Exception e) {
				e.printStackTrace();
				logger.warning("Error sending ZRTP packet to wire: " + e.getMessage());
			}
		}
		
		void sendPacket(RtpPacket pkt) {
			synchronized(pkts) {
				pkts.add(pkt);
			}
		}
		
		synchronized void sendPackets() throws IOException {
			RtpPacket pkt = null;
			while(true) {
				synchronized(pkts) {
					pkt = pkts.poll();
				}
				if(pkt == null)
					return;
				rtp_socket.send(pkt);
			}
		}
		
		// The ZRTP spec requires us to calculate a CRC32C checksum across
		// the whole packet (include RTP header section) and append it to
		// the end of the packet
		protected void appendCrc(RtpPacket packet) {
			int originalLength = packet.getLength();
			byte[] _packet = packet.getPacket();
			CRC32C delegate = new CRC32C();
	        delegate.update(_packet, 0, originalLength);
	        byte[] crc32c = delegate.getValueAsBytes(); 
	        packet.setLength(originalLength + 4);
	        for(int i = 0; i < 4; i++) {
        		_packet[originalLength + i] = crc32c[i];
	        }
		}

		@Override
		public void setNextZrtpSequenceNumber(int startSeqNum) {
			seqNo = startSeqNum;

		}

		@Override
		public void setMasqueradingDual() {
			// TODO Auto-generated method stub

		}

		@Override
		public void setMasqueradingActive() {
			// TODO Auto-generated method stub

		}
	}

	public void setSRTP(SRTP srtp) {
		this.srtp = srtp;
	}
	
	public void setZRTP(ZRTP zrtp) {
		this.zrtp = zrtp;
		if(zrtp == null)
			zrtpHelper = null;
	}

	public void setSeqNum(int firstSeqNum) {
		this.seqn = firstSeqNum;
	}
}
