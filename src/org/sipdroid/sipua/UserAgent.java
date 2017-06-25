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

package org.sipdroid.sipua;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Calendar;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.ice4j.StackProperties;
import org.ice4j.Transport;
import org.ice4j.TransportAddress;
import org.ice4j.ice.Candidate;
import org.ice4j.ice.CandidateType;
import org.ice4j.ice.Component;
import org.ice4j.ice.IceMediaStream;
import org.ice4j.ice.IceProcessingState;
import org.ice4j.ice.LocalCandidate;
import org.ice4j.ice.NominationStrategy;
import org.ice4j.ice.RemoteCandidate;
import org.ice4j.ice.harvest.StunCandidateHarvester;
import org.ice4j.ice.harvest.TurnCandidateHarvester;
import org.ice4j.security.LongTermCredential;
import org.ice4j.socket.DelegatingDatagramSocket;
import org.lumicall.android.R;
import org.lumicall.android.db.SecurityMode;
import zorg.SRTP;
import zorg.ZRTP;
import org.opentelecoms.util.dns.SRVRecordHelper;
import org.sipdroid.codecs.Codec;
import org.sipdroid.codecs.Codecs;
import org.sipdroid.media.JAudioLauncher;
import org.sipdroid.media.MediaLauncher;
import org.sipdroid.media.RtpStreamReceiver;
import org.sipdroid.media.RtpStreamSender;
import org.sipdroid.net.ICESocketAllocator;
import org.sipdroid.net.SipdroidSocket;
import org.sipdroid.net.SipdroidSocketAllocator;
import org.sipdroid.net.SocketAllocator;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;
import org.sipdroid.sipua.ui.Sipdroid;
import org.zoolu.net.IpAddress;
import org.zoolu.sdp.AttributeField;
import org.zoolu.sdp.ConnectionField;
import org.zoolu.sdp.CryptoField;
import org.zoolu.sdp.MediaDescriptor;
import org.zoolu.sdp.MediaField;
import org.zoolu.sdp.OriginField;
import org.zoolu.sdp.SRTPKeySpec;
import org.zoolu.sdp.SessionDescriptor;
import org.zoolu.sdp.TimeField;
import org.zoolu.sip.address.NameAddress;
import org.zoolu.sip.call.Call;
import org.zoolu.sip.call.CallListenerAdapter;
import org.zoolu.sip.call.ExtendedCall;
import org.zoolu.sip.call.SdpTools;
import org.zoolu.sip.header.StatusLine;
import org.zoolu.sip.message.Message;
import org.zoolu.sip.provider.SipProvider;
import org.zoolu.sip.provider.SipStack;
import org.zoolu.tools.Log;
import org.zoolu.tools.LogLevel;
import org.zoolu.tools.Parser;


import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.preference.PreferenceManager;

/**
 * Simple SIP user agent (UA). It includes audio/video applications.
 * <p>
 * It can use external audio/video tools as media applications. Currently only
 * RAT (Robust Audio Tool) and VIC are supported as external applications.
 */
public class UserAgent extends CallListenerAdapter {
	/** Event logger. */
	private Logger logger = Logger.getLogger(getClass().getCanonicalName());
	
	/** Factory/caching system for UDP sockets */
	final static int DEFAULT_RECEIVE_BUFFER_SIZE = 2048;

	// FIXME - to go in ICE module
	final static String ATTR_ICE_UFRAG = "ice-ufrag";
	final static String ATTR_ICE_PWD = "ice-pwd";
	final static String ATTR_ICE_LITE = "ice-lite";
	final static String ATTR_ICE_MISMATCH = "ice-mismatch";
	
	/** Factory/caching system for UDP sockets */
	SocketAllocator socketAllocator = null;

	/** UserAgentProfile */
	public UserAgentProfile user_profile;

	/** SipProvider */
	protected SipProvider sip_provider;

	/** Call */
	// Call call;
	protected ExtendedCall call;

	/** Call transfer */
	protected ExtendedCall call_transfer;

	/** Audio application */
	public MediaLauncher audio_app = null;

	/** Local sdp */
	protected String local_session = null;
	
	public static final int UA_STATE_IDLE = 0;
	public static final int UA_STATE_INCOMING_CALL = 1;
	public static final int UA_STATE_OUTGOING_CALL = 2;
	public static final int UA_STATE_INCALL = 3;
	public static final int UA_STATE_HOLD = 4;

	private static final String SUPPORTED_CRYPTO_SUITE = "AES_CM_128_HMAC_SHA1_80";
	public static final int SUPPORTED_CRYPTO_SUITE_TAG_SIZE = 10;

	int call_state = UA_STATE_IDLE;
	String remote_media_address;
	int remote_video_port,local_video_port;

	// *************************** Basic methods ***************************

	/** Changes the call state */
	protected synchronized void changeStatus(int state,String caller) {
		call_state = state;
		Receiver.onState(state, caller);
		if(state == UA_STATE_IDLE) {
			peerUserAgent = null;
			setSocketAllocator(null);
			if(iceAgent != null) {
				iceAgent.free();
				iceAgent = null;
			}
			if(mediaStreams != null) {
				mediaStreams.clear();
			}
		}
	}
	
	protected void changeStatus(int state) {
		changeStatus(state, null);
	}

	/** Checks the call state */
	protected boolean statusIs(int state) {
		return (call_state == state);
	}

	/**
	 * Sets the automatic answer time (default is -1 that means no auto accept
	 * mode)
	 */
	public void setAcceptTime(int accept_time) {
		user_profile.accept_time = accept_time;
	}

	/**
	 * Sets the automatic hangup time (default is 0, that corresponds to manual
	 * hangup mode)
	 */
	public void setHangupTime(int time) {
		user_profile.hangup_time = time;
	}

	/** Sets the redirection url (default is null, that is no redircetion) */
	public void setRedirection(String url) {
		user_profile.redirect_to = url;
	}

	/** Sets the no offer mode for the invite (default is false) */
	public void setNoOfferMode(boolean nooffer) {
		user_profile.no_offer = nooffer;
	}

	/** Enables audio */
	public void setAudio(boolean enable) {
		user_profile.audio = enable;
	}

	/** Sets the receive only mode */
	public void setReceiveOnlyMode(boolean r_only) {
		user_profile.recv_only = r_only;
	}

	/** Sets the send only mode */
	public void setSendOnlyMode(boolean s_only) {
		user_profile.send_only = s_only;
	}

	/** Sets the send tone mode */
	public void setSendToneMode(boolean s_tone) {
		user_profile.send_tone = s_tone;
	}

	/** Sets the send file */
	
	public void setSendFile(String file_name) {
		user_profile.send_file = file_name;
	}

	/** Sets the recv file */
	
	public void setRecvFile(String file_name) {
		user_profile.recv_file = file_name;
	}
	
	/** Gets the local SDP */
	public String getSessionDescriptor() {
		return local_session;
	}

	//change start (multi codecs)
	/** Inits the local SDP (no media spec) */
	public void initSessionDescriptor(Codecs.Map c) {
		SessionDescriptor sdp = new SessionDescriptor(
				user_profile.from_url,
				sip_provider.getViaAddress());
		
		if(iceAgent != null) {
			sdp.addAttribute(new AttributeField(ATTR_ICE_UFRAG, iceAgent.getLocalUfrag()));
			sdp.addAttribute(new AttributeField(ATTR_ICE_PWD, iceAgent.getLocalPassword()));
		} else {
			printLog("not adding ICE data to SDP (iceAgent == null)");
		}
		
		local_session = sdp.toString();
		
		//We will have at least one media line, and it will be 
		//audio
		if (user_profile.audio || !user_profile.video)
		{
//			addMediaDescriptor("audio", user_profile.audio_port, c, user_profile.audio_sample_rate);
			addMediaDescriptor("audio", user_profile.audio_port, c);
		}
		
		if (user_profile.video)
		{
			addMediaDescriptor("video", user_profile.video_port,
					user_profile.video_avp, "h263-1998", 90000);
		}
		
		if(iceAgent != null) {
			updateLocalSDPFromICEAgent();
		}
	}
	//change end
	
	/** Adds a single media to the SDP */
	private void addMediaDescriptor(String media, int port, int avp,
					String codec, int rate) {
		SessionDescriptor sdp = new SessionDescriptor(local_session);
		
		String attr_param = String.valueOf(avp);
		
		if (codec != null)
		{
			attr_param += " " + codec + "/" + rate;
		}
		Vector<AttributeField> afvec = new Vector<AttributeField>();
		
		afvec.add(new AttributeField("rtpmap", attr_param));
		
		if(iceAgent != null)
			addICECandidates(afvec, port, media);
		
		sdp.addMedia(new MediaField(media, port, 0, "RTP/AVP", 
				String.valueOf(avp)), afvec);
		
		local_session = sdp.toString();
	}
	
	/** Adds a set of media to the SDP */
//	private void addMediaDescriptor(String media, int port, Codecs.Map c,int rate) {
	private void addMediaDescriptor(String media, int port, Codecs.Map c) {
		SessionDescriptor sdp = new SessionDescriptor(local_session);
	
		Vector<String> avpvec = new Vector<String>();
		Vector<AttributeField> afvec = new Vector<AttributeField>();
		if (c == null) {
			// offer all known codecs
			for (int i : Codecs.getCodecs()) {
				Codec codec = Codecs.get(i);
				if (i == 0) codec.init();
				avpvec.add(String.valueOf(i));
				if (codec.number() == 9)
					afvec.add(new AttributeField("rtpmap", String.format("%d %s/%d", i, codec.userName(), 8000))); // kludge for G722. See RFC3551.
				else if (codec.userName().equals("opus"))
					afvec.add(new AttributeField("rtpmap", String.format("%d %s/%d/%d", i, codec.userName(), 48000, 2)));
				else
					afvec.add(new AttributeField("rtpmap", String.format("%d %s/%d", i, codec.userName(), codec.samp_rate())));
			}
		} else {
			c.codec.init();
			avpvec.add(String.valueOf(c.number));
			if (c.codec.number() == 9)
				afvec.add(new AttributeField("rtpmap", String.format("%d %s/%d", c.number, c.codec.userName(), 8000))); // kludge for G722. See RFC3551.
			else if (c.codec.userName().equals("opus"))
                                afvec.add(new AttributeField("rtpmap", String.format("%d %s/%d/%d", c.number, c.codec.userName(), 48000, 2)));
			else
				afvec.add(new AttributeField("rtpmap", String.format("%d %s/%d", c.number, c.codec.userName(), c.codec.samp_rate())));
		}
		if (user_profile.dtmf_avp != 0){
			avpvec.add(String.valueOf(user_profile.dtmf_avp));
			afvec.add(new AttributeField("rtpmap", String.format("%d telephone-event/%d", user_profile.dtmf_avp, user_profile.audio_sample_rate)));
			afvec.add(new AttributeField("fmtp", String.format("%d 0-15", user_profile.dtmf_avp)));
		}
				
		//String attr_param = String.valueOf(avp);
		
		if(iceAgent != null)
			addICECandidates(afvec, port, media);
		
		String mediaProfile = "RTP/AVP";
		if(user_profile.security_mode == SecurityMode.SRTP) {
			mediaProfile = "RTP/SAVP";  // default secure profile, but...
			String _rsdp = call.getRemoteSessionDescriptor();
			if(_rsdp != null) {
				SessionDescriptor remote_sdp = new SessionDescriptor(_rsdp);
				MediaDescriptor _rmd = remote_sdp.getMediaDescriptor(media);
				if(_rmd != null) {
					if(_rmd.getMedia().getTransport().equals("RTP/SAVPF"))
						mediaProfile = "RTP/SAVPF";
				}
			}
			int secItem = 1;
			String secSuite = SUPPORTED_CRYPTO_SUITE;
			String secKey = SRTPKeySpec.generate().toString();
			afvec.add(new AttributeField("crypto",
					String.format("%d %s inline:%s", secItem, secSuite, secKey)));
		}
		
		sdp.addMedia(new MediaField(media, port, 0, mediaProfile, avpvec), afvec);
		local_session = sdp.toString();
	}
	
	Map<String, MediaStream> mediaStreams = null;
	
	protected void addICECandidates(Vector<AttributeField> afvec, int port, String media) {
		IceMediaStream ims = null;
		try {
			MediaStream ms = new MediaStream(iceAgent, media, port);
			if(mediaStreams == null)
				mediaStreams = new HashMap<String, MediaStream>();
			mediaStreams.put(media, ms);
			ims = ms.getIceMediaStream();
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			printLog("FAILED TO CREATE STREAM FOR ICE: " + media + ": " + e.getMessage());
			e.printStackTrace();
			return;
		}
		// Now add the candidates for ICE for this media descriptor
		/*
		 *  candidate-attribute   = "candidate" ":" foundation SP component-id SP
                       transport SP
                       priority SP
                       connection-address SP     ;from RFC 4566
                       port         ;port from RFC 4566
                       SP cand-type
                       [SP rel-addr]
                       [SP rel-port]
		 *(SP extension-att-name SP
                            extension-att-value)

			foundation            = 1*32ice-char
			component-id          = 1*5DIGIT
			transport             = "UDP" / transport-extension
			transport-extension   = token              ; from RFC 3261
			priority              = 1*10DIGIT
			cand-type             = "typ" SP candidate-types
			candidate-types       = "host" / "srflx" / "prflx" / "relay" / token
			rel-addr              = "raddr" SP connection-address
			rel-port              = "rport" SP port
			extension-att-name    = byte-string    ;from RFC 4566
			extension-att-value   = byte-string
			ice-char              = ALPHA / DIGIT / "+" / "/"
		 */

		// Sample from the RFC:
		// a=candidate:1 1 UDP 2130706431 10.0.1.1 8998 typ host
		// a=candidate:2 1 UDP 1694498815 192.0.2.3 45664 typ srflx raddr 10.0.1.1 rport 8998

		for(Component c1 : ims.getComponents()) {
			for(LocalCandidate lc : c1.getLocalCandidates()) {
				String candidate = lc.toString();  // returns the candidate in SDP form
				afvec.add(new AttributeField(candidate));
			}
		}
	}
	
	private void updateLocalSDPFromICEAgent() {
		
		SessionDescriptor sdp = new SessionDescriptor(local_session);
		
		IceMediaStream stream = iceAgent.getStreams().get(0);
		TransportAddress defaultAddress = null;
		if(stream != null) {
			defaultAddress = stream.getComponent(Component.RTP)
						.getDefaultCandidate()
	                			.getTransportAddress();
		} else {
			throw new RuntimeException("Failed to find defaultAddress");
		}
		
		printLog("Default candidate = " + defaultAddress);

		String addrType = defaultAddress.isIPv6()
				? "IP6"	: "IP4";

		//origin (use ip from the first component of the first stream)
		OriginField o = new OriginField("user", "0", "0", addrType,
				defaultAddress.getHostAddress());
		sdp.setOrigin(o);

		//connection  (again use ip from first stream's component)
		ConnectionField c = new ConnectionField(addrType,
				defaultAddress.getHostAddress() );
		sdp.setConnection(c);
		
		MediaDescriptor md = sdp.getMediaDescriptor("audio");
		Vector<AttributeField> attrs = md.getAttributes();
		c = md.getConnection();
		ConnectionField c2 = null;
		if(c != null) {
			c = new ConnectionField(addrType, defaultAddress.getHostAddress() );
		}
		MediaField mf = md.getMedia();
		MediaField mf2 = new MediaField(mf.getMedia(), defaultAddress.getPort(), 0, mf.getTransport(), mf.getFormatList());
		md = new MediaDescriptor(mf2, c2, attrs);
		sdp.addMediaDescriptor(md);

		local_session = sdp.toString();
		
	}


	// *************************** Public Methods **************************

	/** Costructs a UA with a default media port */
	public UserAgent(SipProvider sip_provider, UserAgentProfile user_profile) {
		this.sip_provider = sip_provider;
		this.user_profile = user_profile;
		realm = user_profile.realm;
		
		// if no contact_url and/or from_url has been set, create it now
		user_profile.initContactAddress(sip_provider);
	}
	
	org.ice4j.ice.Agent iceAgent = null;
	
	protected void setupICE(NameAddress caller) {
		
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(Receiver.mContext);
		
		//DelegatingDatagramSocket.setDefaultDelegateFactory(new SipdroidSocketFactory());
		SipdroidSocket.enableJNIImpl(false);
		DelegatingDatagramSocket.setDefaultReceiveBufferSize(DEFAULT_RECEIVE_BUFFER_SIZE);
		
		if(sp.getBoolean(Settings.PREF_IPV4, Settings.DEFAULT_IPV4))
			System.setProperty(StackProperties.DISABLE_IPv6, "true");
        System.setProperty(StackProperties.MAX_CTRAN_RETRANS_TIMER, "200");
        System.setProperty(StackProperties.MAX_CTRAN_RETRANSMISSIONS, "10");

		
		iceAgent = new org.ice4j.ice.Agent();
		
		
		
		String iceServer = user_profile.stun_server_name;
		int port = user_profile.stun_server_port;
		if(iceServer != null && iceServer.length() > 0) {
			
			String turnUser = user_profile.username;
			String turnPassword = user_profile.passwd;
		
			LongTermCredential longTermCredential = null;
			
			/**
			 * This is a hack: if we are the callee and the caller is a
			 * sip5060.net user then we assume they have a TURN relay already
			 * and we leave the longTermCredential empty, just using STUN
			 * instead of relay.  This means that when a sip5060.net user
			 * calls another sip5060.net user, only one of them will have a
			 * TURN allocation.
			 *
			 * FIXME: check the SDP to see if the peer really has
			 * a TURN relay, that would be a much safer solution as then we
			 * would know there is always at least one relay.
			 */
			boolean sip5060Caller = (caller != null && caller.getAddress().getHost().equals("sip5060.net"));
			if(turnUser != null && turnPassword != null && !sip5060Caller) {
				printLog("username for TURN: " + turnUser);
				longTermCredential = new LongTermCredential(turnUser, turnPassword);
			}
        
			// FIXME - should happen in parallel
			int iceServerCount = 0;
			if(longTermCredential != null) {
				/* FIXME
				 * If there is an SRV record for UDP and none for TLS, the
				 * default behavior of SRVRecordHelper returns the UDP TURN
				 * server but also returns the A or AAAA record for the TLS
				 * search, using the default port given below.  Results from
				 * A/AAAA lookup should ONLY be used if no SRV records are
				 * found for any protocol.
				 *
				 * The solution involves refactoring the code to use the
				 * algorithm described in RFC 5928
				 * https://tools.ietf.org/html/rfc5928
				 */
				iceServerCount += findIceServers(longTermCredential, iceServer, port, Transport.UDP);
				iceServerCount += findIceServers(longTermCredential, iceServer, port, Transport.TLS);
			}
			if(iceServerCount == 0) {
				logger.warning("no TURN servers found by SRV lookup, looking for STUN servers");
				iceServerCount += findIceServers(null, iceServer, port, Transport.UDP);
			}
			if(iceServerCount == 0) {
				logger.warning("no STUN or TURN servers found");
			}
		}

        //STREAMS
        // should this be done elsewhere, in the SDP creation function?
        //createStream(rtpPort, "audio", agent);
        //createStream(rtpPort + 2, "video", agent);
		
		iceAgent.setNominationStrategy(
                NominationStrategy.NOMINATE_HIGHEST_PRIO);
		
		iceAgent.addStateChangeListener(new IceProcessingListener(caller));
		
		// this should be done after we receive/send SIP 1xx status
		// (depends on whether we are caller/callee)
		//localAgent.setControlling(true);
	}
	

	protected int findIceServers(LongTermCredential ltc, String iceServer, int port, Transport transport) {
		String querySvc = "stun";
		if(ltc != null) {
			querySvc = "turn";
		}
		String queryProto = "udp";
		if(transport == Transport.TLS) {
			querySvc = querySvc + "s";
			queryProto = "tcp";
		}
		
		int count = 0;
		SRVRecordHelper srh = new SRVRecordHelper(querySvc, queryProto, iceServer, port);
		for(InetSocketAddress sa : srh) {
			
			String _iceServer = sa.getHostName();
			int _port = sa.getPort();
			LongTermCredential _ltc = ltc;
			
			int len = _iceServer.length();
			if(len > 0 && _iceServer.charAt(len - 1) == '.')
				_iceServer = _iceServer.substring(0, len - 1);
			
			if(_ltc != null && _iceServer.equals("stun-test.sip5060.net")) {
				_ltc = new LongTermCredential("test", "notasecret");
				logger.info("*** Using TEST credentials ***");
			}
				
			if(_ltc == null) {
				logger.info("Adding STUN server: [" + _iceServer + "]");
				iceAgent.addCandidateHarvester(
						new StunCandidateHarvester(
								new TransportAddress(_iceServer, _port, transport)));
			} else {
				logger.info("Adding TURN server: [" + _iceServer + "]");
				iceAgent.addCandidateHarvester(
						new TurnCandidateHarvester(
								new TransportAddress(_iceServer, _port, transport),
									_ltc));
			}
			count++;
			
		}            
		return count;
			
	}
	
    /**
     * Creates an <tt>IceMediaStream</tt> and adds to it an RTP and and RTCP
     * component.
     *
     * @param rtpPort the port that we should try to bind the RTP component on
     * (the RTCP one would automatically go to rtpPort + 1)
     * @param streamName the name of the stream to create
     * @param agent the <tt>Agent</tt> that should create the stream.
     *
     * @return the newly created <tt>IceMediaStream</tt>.
     * @throws Throwable if anything goes wrong.
     */
    private IceMediaStream createICEStream(int rtpPort,  String streamName)
        throws Throwable
    {
        IceMediaStream stream = iceAgent.createMediaStream(streamName);
        //rtp
        Component rtpComponent = iceAgent.createComponent(
                stream, Transport.UDP, rtpPort, rtpPort, rtpPort + 100);
        int boundRtpPort = rtpComponent.getLocalCandidates().get(0).getTransportAddress().getPort();
        //rtcpComp
        Component rtcpComponent = iceAgent.createComponent(
                stream, Transport.UDP, rtpPort + 1, rtpPort + 1, rtpPort + 101);
        int boundRtcpPort = rtcpComponent.getLocalCandidates().get(0).getTransportAddress().getPort();
        return stream;
    }

	String realm;

	private boolean iceLitePeer;

	private String peerUserAgent = null;
	
	/** Makes a new call (acting as UAC). */
	public boolean call(String target_url, boolean send_anonymous) {
		
		if (Receiver.call_state != UA_STATE_IDLE)
		{
			//We can initiate or terminate a call only when
			//we are in an idle state
			printLog("Call attempted in state" + this.getSessionDescriptor() + " : Failing Request", LogLevel.HIGH);
			return false;
		}
		hangup(); // modified
		changeStatus(UA_STATE_OUTGOING_CALL,target_url);
		
		AsyncTask _call = new AsyncTask() {

			@Override
			protected Object doInBackground(Object... arg0) {
				logger.info("Starting Async call setup phase");
				callAsync((String)arg0[0], false);
				logger.info("Done Async call setup phase");
				return null;
			}
			
		};
		_call.execute(target_url);
		
		return true;
		
	}
	
	protected boolean callAsync(String target_url, boolean send_anonymous) {
		
		String from_url;
		
		if (!send_anonymous)
		{
			from_url = user_profile.from_url;
		}
		else
		{
			from_url = "sip:anonymous@anonymous.invalid";
		}
		
		printLog("begin ICE setup (outbound)");
		setupICE(null);
		//iceAgent.setControlling(true);
		//iceAgent.startConnectivityEstablishment();
		printLog("done ICE setup (outbound)");

		//change start multi codecs
		createOffer();
		//change end
		call = new ExtendedCall(sip_provider, from_url,
				user_profile.contact_url, user_profile.username,
				user_profile.realm, user_profile.passwd, this);
		
		// in case of incomplete url (e.g. only 'user' is present), try to
		// complete it
		if (target_url.indexOf("@") < 0) {
			if (user_profile.realm.equals(Settings.DEFAULT_SERVER))
				target_url = "&" + target_url;
			target_url = target_url + "@" + realm; // modified
		}
		
		// MMTel addition to define MMTel ICSI to be included in INVITE (added by mandrajg)
		String icsi = null;	
		if (user_profile.mmtel == true){
			icsi = "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"";
		}		
		
		target_url = sip_provider.completeNameAddress(target_url).toString();
		
		if (user_profile.no_offer)
		{
			call.call(target_url);
		}
		else
		{
			call.call(target_url, local_session, icsi);		// modified by mandrajg
		}
		
		return true;
	}

	public void info(char c, int duration)
	{
		boolean use2833 = audio_app != null && audio_app.sendDTMF(c); // send out-band DTMF (rfc2833) if supported

		if (!use2833 && call != null)
		{
			call.info(c, duration);
			logger.info("sent DTMF code '" + c + "' using SIP INFO");
		}
		else if(use2833)
		{
			logger.info("sent DTMF code '" + c + "' using RFC2833");
		}
		else
		{
			logger.warning("failed to send DTMF code '" + c + "'");
		}
	}
	
	/** Waits for an incoming call (acting as UAS). */
	public boolean listen() {
		
		if (Receiver.call_state != UA_STATE_IDLE)
		{
			//We can listen for a call only when
			//we are in an idle state
			printLog("Call listening mode initiated in " + this.getSessionDescriptor() + " : Failing Request", LogLevel.HIGH);
			return false;
		}
		
		hangup();
		
		call = new ExtendedCall(sip_provider, user_profile.from_url,
				user_profile.contact_url, user_profile.username,
				user_profile.realm, user_profile.passwd, this);
		call.listen();
		
		return true;
	}

	/** Closes an ongoing, incoming, or pending call */
	public void hangup() 
	{
		printLog("HANGUP");
		closeMediaApplication();
		
		if (call != null)
		{
			call.hangup();
		}
		
		changeStatus(UA_STATE_IDLE);
	}

	/** Accepts an incoming call */
	public boolean accept() 
	{
		if (call == null)
		{
			return false;
		}
		
		printLog("ACCEPT");
		changeStatus(UA_STATE_INCALL); // modified

		call.accept(local_session);
		launchMediaApplication();
		
		return true;
	}

	/** Redirects an incoming call */
	public void redirect(String redirection) 
	{
		if (call != null)
		{
			call.redirect(redirection);
		}
	}
	
	protected boolean isCompatibleUserAgent() {
		return (peerUserAgent  != null && 
				peerUserAgent.startsWith(Receiver.mContext.getString(R.string.app_name)));
	}

	/** Launches the Media Application (currently, the RAT audio tool) */
	protected void launchMediaApplication() {
		// exit if the Media Application is already running
		if (audio_app != null) {
			// This can happen if it was launched to handle early media SIP 183
			printLog("DEBUG: media application is already running",
					LogLevel.HIGH);
			return;
		}
		Codecs.Map c;
		printLog("AUDIO:  **** LAUNCHING AUDIO APP ****");
		
		// parse local sdp
		SessionDescriptor local_sdp = new SessionDescriptor(call
				.getLocalSessionDescriptor());
		int local_audio_port = 0;
		local_video_port = 0;
		int dtmf_pt = 0;
		c = Codecs.getCodec(local_sdp);
		if (c == null) {
			Receiver.call_end_reason = R.string.card_title_ended_no_codec;
			hangup();
			return;
		}
		MediaDescriptor m = local_sdp.getMediaDescriptor("video");
		if ( m != null)
			local_video_port = m.getMedia().getPort();
		m = local_sdp.getMediaDescriptor("audio");
		if (m != null) {
			if(mediaStreams != null && mediaStreams.get("audio") != null)
				local_audio_port = mediaStreams.get("audio").getRtpPort();
			else
				local_audio_port = m.getMedia().getPort();
			if (m.getMedia().getFormatList().contains(String.valueOf(user_profile.dtmf_avp)))
				dtmf_pt = user_profile.dtmf_avp;
		}
		// parse remote sdp
		SessionDescriptor remote_sdp = new SessionDescriptor(call
				.getRemoteSessionDescriptor());
		remote_media_address = (new Parser(remote_sdp.getConnection()
				.toString())).skipString().skipString().getString();
		int remote_audio_port = 0;
		remote_video_port = 0;
		
		// FIXME - we should avoid making excessive copies of the keys in RAM
		Map<String, SRTPKeySpec> remoteKeys = new HashMap<String, SRTPKeySpec>();
		boolean mustEncrypt = false;
		for (Enumeration<MediaDescriptor> e = remote_sdp.getMediaDescriptors()
				.elements(); e.hasMoreElements();) {
			MediaDescriptor md = e.nextElement();
			MediaField media = md.getMedia();
			if (media.getMedia().equals("audio"))
				remote_audio_port = media.getPort();
			if (media.getMedia().equals("video"))
				remote_video_port = media.getPort();
			String mt = media.getTransport();
			if(mt.equals("RTP/SAVP") || mt.equals("RTP/SAVPF")) {
				mustEncrypt = true;
				for(AttributeField af : md.getAttributes("crypto")) {
					CryptoField cf = new CryptoField(af);
					if(cf.getSuite().equals(SUPPORTED_CRYPTO_SUITE)) {
						SRTPKeySpec keySpec = new SRTPKeySpec(cf.getKey());
						remoteKeys.put(media.getMedia(), keySpec);
					}
				}
			}
		}
		
		Map<String, SRTPKeySpec> localKeys = new HashMap<String, SRTPKeySpec>();
		for(MediaDescriptor md : local_sdp.getMediaDescriptors()) {
			MediaField media = md.getMedia();
			String mt = media.getTransport();
			if(mt.equals("RTP/SAVP") || mt.equals("RTP/SAVPF")) {
				mustEncrypt = true;
				CryptoField cf = new CryptoField(md.getAttribute("crypto"));
				if(cf.getSuite().equals(SUPPORTED_CRYPTO_SUITE)) {
					SRTPKeySpec keySpec = new SRTPKeySpec(cf.getKey());
					localKeys.put(media.getMedia(), keySpec);
				}
			}
		}
		
		SRTP srtp = null;
		ZRTP zrtp = null;
		if(mustEncrypt == true) {
			SRTPKeySpec txAudioKey = localKeys.get("audio");
			//printLog("Using TX key: " + txAudioKey);
			SRTPKeySpec rxAudioKey = remoteKeys.get("audio");
			//printLog("Using RX key: " + rxAudioKey);
			if(txAudioKey == null || rxAudioKey == null)
				throw new RuntimeException("RTP/SAVP(F) detected in SDP, but insufficient crypto keys");
			srtp = new SRTP(new zorg.platform.j2se.PlatformImpl());
			if(!srtp.testEncryption())
				throw new RuntimeException("SRTP.testEncryption() failed, platform not compatible");
			if(!srtp.testReplayCheckVector())
				throw new RuntimeException("SRTP.testReplayCheckVector() failed, platform not compatible");
			srtp.setKDR(48);
			srtp.setHmacAuthSizeBytes(SUPPORTED_CRYPTO_SUITE_TAG_SIZE);  // FIXME
			srtp.setFirstRtpSeqNum(RtpStreamSender.FIRST_SEQ_NUM);  // From RTP Sender
			srtp.setTxMasterKey(txAudioKey.getMaster());
			srtp.setTxMasterSalt(txAudioKey.getSalt());
			srtp.setRxMasterKey(rxAudioKey.getMaster());
			srtp.setRxMasterSalt(rxAudioKey.getSalt());
			if(srtp.startNewSession() != SRTP.SESSION_OK) {
				throw new RuntimeException("Failed to start SRTP session");
			}
		} else if (user_profile.security_mode.equals(SecurityMode.ZRTP)) {
			// User wants to try ZRTP
			// We know that this ZRTP implementation only works with itself and has trouble
			// talking to other implementations such as Jitsi, so let's make sure
			// we are talking to another Lumicall
			if(isCompatibleUserAgent()) {
				zrtp = new ZRTP(new zorg.platform.j2se.PlatformImpl());
				zrtp.setPhoneNumber("+999999999");  // FIXME ZRTP - E.164 number not in use
			} else {
				printLog("not trying ZRTP, peer User-Agent (" + peerUserAgent + ") not known to be compatible");
			}
		}
		
		DatagramSocket socket = null;
		if(mediaStreams != null && mediaStreams.get("audio") != null) {
			int _remote_audio_port = mediaStreams.get("audio").getRemoteRtpPort();
			if(_remote_audio_port > 0)
				remote_audio_port = _remote_audio_port;
			else {
				printLog("*** Remote audio port not found, maybe ICE not finished??? ***");
			}
			String _remote_media_address = mediaStreams.get("audio").getRemoteRtpAddress();
			if(_remote_media_address != null)
				remote_media_address = _remote_media_address;
			else {
				printLog("*** Remote audio address not found, maybe ICE not finished??? ***");
			}
			socket = mediaStreams.get("audio").getRTPSocket();
		} else {
			try {
				socket = getSocketAllocator().allocateSocket(local_audio_port);
			} catch (SocketException e) {
				printException(e, LogLevel.HIGH);
				return;
			} catch (UnknownHostException e) {
				printException(e, LogLevel.HIGH);
				return;
			}
		}

		// select the media direction (send_only, recv_ony, fullduplex)
		int dir = 0;
		if (user_profile.recv_only)
			dir = -1;
		else if (user_profile.send_only)
			dir = 1;

		if (user_profile.audio && local_audio_port != 0
				&& remote_audio_port != 0) { // create an audio_app and start
												// it

			if (audio_app == null) { // for testing..
				String audio_in = null;
				if (user_profile.send_tone) {
					audio_in = JAudioLauncher.TONE;
				} else if (user_profile.send_file != null) {
					audio_in = user_profile.send_file;
				}
				String audio_out = null;
				if (user_profile.recv_file != null) {
					audio_out = user_profile.recv_file;
				}
				
				printLog("AUDIO PARAMS: Local port: " + local_audio_port + ", Remote: " + remote_media_address + ", " + remote_audio_port);

				audio_app = new JAudioLauncher(socket, local_audio_port,
						remote_media_address, remote_audio_port, dir, audio_in,
						audio_out, c.codec.samp_rate(),
						user_profile.audio_sample_size,
						c.codec.frame_size(), c, dtmf_pt, srtp, zrtp);
			}
			audio_app.startMedia();
		}
	}

	/** Close the Media Application */
	protected void closeMediaApplication() {
		if (audio_app != null) {
			audio_app.stopMedia();
			audio_app = null;
		}
	}
	
	public boolean muteMediaApplication() {
		if (audio_app != null)
			return audio_app.muteMedia();
		return false;
	}

	public int speakerMediaApplication(int mode) {
		int old;
		
		if (audio_app != null)
			return audio_app.speakerMedia(mode);
		old = RtpStreamReceiver.speakermode;
		RtpStreamReceiver.speakermode = mode;
		return old;
	}

	public void bluetoothMediaApplication() {
		if (audio_app != null)
			audio_app.bluetoothMedia();
	}

	private void createOffer() {
		initSessionDescriptor(null);
	}

	private void createAnswer(SessionDescriptor remote_sdp) {

		Codecs.Map c = Codecs.getCodec(remote_sdp);
		if (c == null)
			throw new RuntimeException("Failed to get CODEC: AVAILABLE : " + remote_sdp);
		initSessionDescriptor(c);
		sessionProduct(remote_sdp);
	}

	private void sessionProduct(SessionDescriptor remote_sdp) {
		SessionDescriptor local_sdp = new SessionDescriptor(local_session);
		AttributeField iceUFrag = local_sdp.getAttribute(ATTR_ICE_UFRAG);
		AttributeField icePwd = local_sdp.getAttribute(ATTR_ICE_PWD);
		SessionDescriptor new_sdp = new SessionDescriptor(local_sdp
				.getOrigin(), local_sdp.getSessionName(), local_sdp
				.getConnection(), local_sdp.getTime());
		new_sdp.addMediaDescriptors(local_sdp.getMediaDescriptors());
		new_sdp = SdpTools.sdpMediaProduct(new_sdp, remote_sdp
				.getMediaDescriptors());
		if(iceUFrag != null) {
			new_sdp.addAttribute(iceUFrag);
			new_sdp.addAttribute(icePwd);
		}
		//new_sdp = SdpTools.sdpAttirbuteSelection(new_sdp, "rtpmap"); ////change multi codecs
		local_session = new_sdp.toString();
		if (call!=null) call.setLocalSessionDescriptor(local_session);
	}
	
	String[] getRemoteICEAttrs(SessionDescriptor _sdp) {
		String[] result = new String[] { null, null };
		
		if(_sdp.getAttribute(ATTR_ICE_UFRAG) != null)
			result[0] = _sdp.getAttribute(ATTR_ICE_UFRAG).getAttributeValue();
		if(_sdp.getAttribute(ATTR_ICE_PWD) != null)
			result[1] = _sdp.getAttribute(ATTR_ICE_PWD).getAttributeValue();
		
		MediaDescriptor mAudio = _sdp.getMediaDescriptor("audio");
		if(mAudio != null) {
			if(mAudio.getAttribute(ATTR_ICE_UFRAG) != null)
				result[0] = mAudio.getAttribute(ATTR_ICE_UFRAG).getAttributeValue();
			if(mAudio.getAttribute(ATTR_ICE_PWD) != null)
				result[1] = mAudio.getAttribute(ATTR_ICE_PWD).getAttributeValue();
		}
		
		if(result[0] == null || result[1] == null)
			result = null;
		return result;
	}
	
	void handleRemoteSDPforICE(SessionDescriptor remote_sdp) {
		
		printLog("scanning for ICE candidates in remote SDP");
		printLog("local stream count = " + iceAgent.getStreamCount());
		
		if(remote_sdp.hasAttribute(ATTR_ICE_MISMATCH)) {
			printLog("ice-mismatch reported by peer, aborting");
			Receiver.call_end_reason = R.string.card_title_ended_ICE_failure;
			changeStatus(UA_STATE_IDLE);
			return;
		}
			
		if(remote_sdp.hasAttribute(ATTR_ICE_LITE)) {
			// peer is ICE-lite
			iceLitePeer = true;
			// According to RFC 5245, we must be the controlling agent
			// if the peer is a LITE implementation
			iceAgent.setControlling(true);
			printLog("peer is an ICE LITE implementation, we are the controlling agent");
		} else {
			iceLitePeer = false;
			printLog("peer is not an ICE LITE implementation");
		}
		
		String[] iceAttrs = getRemoteICEAttrs(remote_sdp);
		String rUfrag = iceAttrs[0];
		String rPassword = iceAttrs[1];
		
		ConnectionField globalConnection = remote_sdp.getConnection();
        String globalConnAddr = null;
        if(globalConnection != null)
            globalConnAddr = globalConnection.getAddress();
		
		// Now map the remote candidates into the local ICE agent
		for(IceMediaStream stream : iceAgent.getStreams())
		{
			String streamName = stream.getName();
			printLog("scanning SDP for ICE candidates for stream: " + streamName);
			
			// seems that we need to do this before adding remote candidates or we
			// get an exception
			stream.setRemoteUfrag(rUfrag);
			stream.setRemotePassword(rPassword);
			
			MediaDescriptor md = remote_sdp.getMediaDescriptor(streamName);
			if(md != null) {
				
				
				
				Vector<AttributeField> afvec = md.getAttributes("candidate");
				printLog("handling ICE SDP for stream: " + streamName + ", candidate count = " + afvec.size());
				
				for(AttributeField af : afvec) {
					String candidate = af.getAttributeValue();
					StringTokenizer st = new StringTokenizer(candidate);
					int nTokens = st.countTokens();
					if (nTokens  < 8 ) {
						printLog("BAD CANDIDATE (" + nTokens + " tokens) [" + candidate + "]");
						return;
					}
				    String foundation = st.nextToken();
				    String componentId = st.nextToken();
				    String protocol = st.nextToken().toLowerCase();  // FIXME bug in ice4j - expects lowercase udp
				    String priority = st.nextToken();
				    String addr = st.nextToken();
				    String port = st.nextToken();
				    st.nextToken(); // typ
				    String type = st.nextToken();
				    String rAddr = null;
				    String rPort = null;
				    if(st.countTokens() == 12) {
				    	st.nextToken();
				    	rAddr = st.nextToken();
				    	st.nextToken();
				    	rPort = st.nextToken();
				    } else {
				    	logger.warning("Ignoring some tokens from candidate: [" + candidate + "]");
				    }
				    
				    Component lc = stream.getComponent(Integer.parseInt(componentId));
				    
				    RemoteCandidate rc = null;
				    RemoteCandidate relatedCandidate = null;
					try {
						if(rAddr != null) {
							TransportAddress relatedTransport =
									new TransportAddress(InetAddress.getByName(rAddr),
											Integer.parseInt(rPort),
											Transport.parse(protocol));
							relatedCandidate = lc.findRemoteCandidate(relatedTransport);
						}
						rc = new RemoteCandidate(
								new TransportAddress(InetAddress.getByName(addr),
										Integer.parseInt(port),
										Transport.parse(protocol)),
								lc,
								CandidateType.parse(type),
								foundation,
								Long.parseLong(priority),
								relatedCandidate
								);
						lc.addRemoteCandidate(rc);
					} catch (Exception e) {
						// FIXME TODO Auto-generated catch block
						printLog("EXCEPTION: " + e.getMessage());
						e.printStackTrace();
						return;
					} 
				    
				} // for (all candidate attribute lines)
				
				// Identify the `default' candidate as per the ICE spec
				ConnectionField streamConnection = md.getConnection();
	            String streamConnAddr = null;
	            if(streamConnection != null)
	                streamConnAddr = streamConnection.getAddress();
	            else
	                streamConnAddr = globalConnAddr;
	            int port = md.getMedia().getPort();
	            TransportAddress defaultRtpAddress =
	                    new TransportAddress(streamConnAddr, port, Transport.UDP);
	            Component rtpComponent = stream.getComponent(Component.RTP);
	            Candidate defaultRtpCandidate
                	= rtpComponent.findRemoteCandidate(defaultRtpAddress);
	            rtpComponent.setDefaultRemoteCandidate(defaultRtpCandidate);
				
			} else {
				iceAgent.removeStream(stream);
			}
		}
		
		printLog("done ICE setup (incoming)");
	}
	
	public void setSocketAllocator(SocketAllocator sa) {
		if(socketAllocator != null) {
			printLog("Destroying old SocketAllocator: " + socketAllocator.getClass().getCanonicalName());
			socketAllocator.free();
			socketAllocator = null;
		}
		printLog("setting SocketAllocator: " + (sa == null ? "<null>" : sa.getClass().getCanonicalName()));
		socketAllocator = sa;
	}


	public SocketAllocator getSocketAllocator() {
		if(socketAllocator == null) {
			setSocketAllocator(new SipdroidSocketAllocator());
		}
		return socketAllocator;
	}


	// ********************** Call callback functions **********************

	/**
	 * Callback function called when arriving a new INVITE method (incoming
	 * call)
	 */
	public void onCallIncoming(Call call, NameAddress callee,
			NameAddress caller, String sdp, Message invite) {
		printLog("onCallIncoming()", LogLevel.LOW);
		
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("INCOMING", LogLevel.HIGH);
		int i = 0;
		for (UserAgent ua : Receiver.mSipdroidEngine.uas) {
			if (ua == this) break;
			i++;
		}
		if (Receiver.call_state != UA_STATE_IDLE || !Receiver.isFast(i)) {
			call.busy();
			listen();
			return;
		}
		
		updatePeerUserAgent(invite);
		
		/* if (Receiver.mSipdroidEngine != null)  // FIXME - moved for ICE, must review
			Receiver.mSipdroidEngine.ua = this;
		changeStatus(UA_STATE_INCOMING_CALL,caller.toString()); */

		if (sdp == null) {
			try {
				createOffer();
			} catch (Exception e) {
                                // only known exception is no local IP found
                                Receiver.call_end_reason = R.string.card_title_ended_unknown;
                                printException(e, LogLevel.HIGH);
                                changeStatus(UA_STATE_IDLE);
                                return;
                        }
		}
		else { 
			SessionDescriptor remote_sdp = new SessionDescriptor(sdp);
			if(getRemoteICEAttrs(remote_sdp) != null) {
				printLog("begin ICE setup (incoming)");
				setupICE(caller);
				iceAgent.setControlling(false);
				//cacheCaller = caller;
			}
			int failure_reason = R.string.card_title_ended_no_codec;
			try {
				// FIXME We should do it like this:
				// 1) create local streams
				// 2) match streams to remotes
				// 3) create answer
				createAnswer(remote_sdp);
				failure_reason = R.string.card_title_ended_ICE_failure;
				if(iceAgent != null) {
					handleRemoteSDPforICE(remote_sdp);
					call.respondProvisional(local_session);
					iceAgent.startConnectivityEstablishment();
				}
			} catch (Exception e) {
				// only known exception is no codec
				// FIXME - could also be ICE trouble
				Receiver.call_end_reason = failure_reason;
				printException(e, LogLevel.HIGH);
				changeStatus(UA_STATE_IDLE);
				return;
			}
		}
		
		if(iceAgent == null) {
			proceedToRing(caller);
		}
	}
	
	private void updatePeerUserAgent(Message msg) {
		if(msg.hasUserAgentHeader()) {
			String _ua = msg.getUserAgentHeader().getInfo();
			if(_ua != null)
				peerUserAgent = _ua;
		} else if(msg.hasServerHeader()) {
			String _server = msg.getServerHeader().getInfo();
			if(_server != null)
				peerUserAgent = _server;
		}
	}

	protected void proceedToRing(NameAddress caller) {
		if (Receiver.mSipdroidEngine != null)
			Receiver.mSipdroidEngine.ua = this;
		changeStatus(UA_STATE_INCOMING_CALL, caller.toString());
		//call.ring(local_session);
		call.ring(null);  // just send 180, no SDP
		//launchMediaApplication();
	}	
	
	public class IceProcessingListener implements PropertyChangeListener {
		
		ICESocketAllocator tmp = null;
		NameAddress caller = null;

		public IceProcessingListener(NameAddress caller) {
			this.caller = caller;
		}
		
		@Override
		public void propertyChange(PropertyChangeEvent event) {
			IceProcessingState iceProcessingState = (IceProcessingState)event.getNewValue();
			
			printLog("IceProcessingListener: received event: " + iceProcessingState);
			
			switch (iceProcessingState) {
			case RUNNING:
				logger.info("received notification that ICE is running");
				break;
			case COMPLETED:
				tmp = new ICESocketAllocator(iceAgent);
				setSocketAllocator(tmp);
				mediaStreams.get("audio").handleCompletion();
				if(Receiver.call_state == UA_STATE_IDLE) {
					// We are the callee - alert the local user, send back 180
					proceedToRing(caller);
				} else if(Receiver.call_state == UA_STATE_INCALL) {
					mediaStreams.get("audio").handleAnswer();
					launchMediaApplication();
				} else {
					// We are the caller - maybe show an ICE completed message?
				}
				break;
			case TERMINATED:
				// We don't currently do anything on the TERMINATED event.
				break;
			case FAILED:
				Receiver.call_end_reason = R.string.card_title_ended_ICE_failure;
				if(Receiver.call_state == UA_STATE_OUTGOING_CALL)
					call.hangup();
				changeStatus(UA_STATE_IDLE);
				if (call != null) {
					call.listen();
				}
				break;
			default:
				logger.warning("IceProcessingListener: unhandled event: " + iceProcessingState);
				return;
			}
			
		}
		
	}
	

	/**
	 * Callback function called when arriving a new Re-INVITE method
	 * (re-inviting/call modify)
	 */
	public void onCallModifying(Call call, String sdp, Message invite) 
	{
		printLog("onCallModifying()", LogLevel.LOW);
		if (call != this.call) 
		{
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("RE-INVITE/MODIFY", LogLevel.HIGH);

		// to be implemented.
		// currently it simply accepts the session changes (see method
		// onCallModifying() in CallListenerAdapter)
		super.onCallModifying(call, sdp, invite);
	}

	/**
	 * Callback function that may be overloaded (extended). Called when arriving
	 * a 180 Ringing or a 183 Session progress with SDP 
	 */
	public void onCallRinging(Call call, Message resp) {
		printLog("onCallRinging(), Status-Line = " + resp.getStatusLine(), LogLevel.LOW);
		if (call != this.call && call != call_transfer) 
		{
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		
		updatePeerUserAgent(resp);
		
		String _remote_sdp = call.getRemoteSessionDescriptor();
		if (_remote_sdp==null || _remote_sdp.length()==0 ||
				!resp.hasBody() || resp.getBody() == null ||
				resp.getBody().length() == 0) {
			printLog("RINGING", LogLevel.HIGH);
			RtpStreamReceiver.ringback(true);
		}
		else {
			printLog("RING/PROVISIONAL (with SDP)", LogLevel.HIGH);
			SessionDescriptor remote_sdp = new SessionDescriptor(_remote_sdp);
			// Maybe ICE?
			IceProcessingState iceProcessingState = null;
			if(getRemoteICEAttrs(remote_sdp) != null && iceAgent != null) {
				iceProcessingState = iceAgent.getState();
				if(iceProcessingState == IceProcessingState.WAITING) {
					handleRemoteSDPforICE(remote_sdp);
					iceAgent.setControlling(true);
					iceAgent.startConnectivityEstablishment();
				}
			} else {
				if (! user_profile.no_offer) {
				
					printLog("Provisional SDP/must receive media");
					RtpStreamReceiver.ringback(false);
					// Update the local SDP along with offer/answer 
					sessionProduct(remote_sdp);
					
					if(iceAgent != null && (
							iceAgent.getState() == IceProcessingState.WAITING ||
							iceAgent.getState() == IceProcessingState.COMPLETED ||
							iceAgent.getState() == IceProcessingState.TERMINATED)) {
						mediaStreams.get("audio").handleAnswer();
					}
				
					launchMediaApplication();
				}
			}
		}
	}

	/** Callback function called when arriving a 2xx (call accepted) */
	public void onCallAccepted(Call call, String sdp, Message resp) 
	{
		printLog("onCallAccepted()", LogLevel.LOW);
		
		if (call != this.call && call != call_transfer) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		
		printLog("ACCEPTED/CALL", LogLevel.HIGH);
		
		if (!statusIs(UA_STATE_OUTGOING_CALL)) { // modified
			hangup();
			return;
		}
		
		updatePeerUserAgent(resp);
		
		changeStatus(UA_STATE_INCALL);
		
		SessionDescriptor remote_sdp = new SessionDescriptor(sdp);
		if (user_profile.no_offer) {
			// answer with the local sdp
			createAnswer(remote_sdp);
			call.ackWithAnswer(local_session);
		} else {
			// Update the local SDP along with offer/answer 
			sessionProduct(remote_sdp);
		}
		if(iceAgent != null && (
				iceAgent.getState() == IceProcessingState.WAITING ||
				iceAgent.getState() == IceProcessingState.COMPLETED ||
				iceAgent.getState() == IceProcessingState.TERMINATED)) {
			mediaStreams.get("audio").handleAnswer();
			launchMediaApplication();
		}

		if (call == call_transfer) 
		{
			StatusLine status_line = resp.getStatusLine();
			int code = status_line.getCode();
			String reason = status_line.getReason();
			this.call.notify(code, reason);
		}
	}

	/** Callback function called when arriving an ACK method (call confirmed) */
	public void onCallConfirmed(Call call, String sdp, Message ack) 
	{
		printLog("onCallConfirmed()", LogLevel.LOW);
	
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		
		printLog("CONFIRMED/CALL", LogLevel.HIGH);

//		changeStatus(UA_STATE_INCALL); modified
		
		if (user_profile.hangup_time > 0)
		{
			this.automaticHangup(user_profile.hangup_time);
		}
	}

	/** Callback function called when arriving a 2xx (re-invite/modify accepted) */
	public void onCallReInviteAccepted(Call call, String sdp, Message resp) {
		printLog("onCallReInviteAccepted()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("RE-INVITE-ACCEPTED/CALL", LogLevel.HIGH);
		if (statusIs(UA_STATE_HOLD))
			changeStatus(UA_STATE_INCALL);
		else
			changeStatus(UA_STATE_HOLD);
	}

	/** Callback function called when arriving a 4xx (re-invite/modify failure) */
	public void onCallReInviteRefused(Call call, String reason, Message resp) {
		printLog("onCallReInviteRefused()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("RE-INVITE-REFUSED (" + reason + ")/CALL", LogLevel.HIGH);
	}

	/** Callback function called when arriving a 4xx (call failure) */
	public void onCallRefused(Call call, String reason, Message resp) {
		printLog("onCallRefused()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("REFUSED (" + reason + ")", LogLevel.HIGH);
		if (reason.equalsIgnoreCase("not acceptable here")) {
			// bummer we have to string compare, this is sdp 488
			Receiver.call_end_reason = R.string.card_title_ended_no_codec;
		}
		changeStatus(UA_STATE_IDLE);
		
		if (call == call_transfer) 
		{
			StatusLine status_line = resp.getStatusLine();
			int code = status_line.getCode();
			// String reason=status_line.getReason();
			this.call.notify(code, reason);
			call_transfer = null;
		}
	}

	/** Callback function called when arriving a 3xx (call redirection) */
	public void onCallRedirection(Call call, String reason,
			Vector<String> contact_list, Message resp) {
		printLog("onCallRedirection()", LogLevel.LOW);
		if (call != this.call) 
		{
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("REDIRECTION (" + reason + ")", LogLevel.HIGH);
		call.call(((String) contact_list.elementAt(0)));
	}

	/**
	 * Callback function that may be overloaded (extended). Called when arriving
	 * a CANCEL request
	 */
	public void onCallCanceling(Call call, Message cancel) {
		printLog("onCallCanceling()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("CANCEL", LogLevel.HIGH);
		changeStatus(UA_STATE_IDLE);
	}

	/** Callback function called when arriving a BYE request */
	public void onCallClosing(Call call, Message bye) {
		printLog("onCallClosing()", LogLevel.LOW);
		if (call != this.call && call != call_transfer) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}

		if (call != call_transfer && call_transfer != null) {
			printLog("CLOSE PREVIOUS CALL", LogLevel.HIGH);
			this.call = call_transfer;
			call_transfer = null;
			return;
		}
		// else
		printLog("CLOSE", LogLevel.HIGH);
		closeMediaApplication();
		changeStatus(UA_STATE_IDLE);
	}

	/**
	 * Callback function called when arriving a response after a BYE request
	 * (call closed)
	 */
	public void onCallClosed(Call call, Message resp) {
		printLog("onCallClosed()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("CLOSE/OK", LogLevel.HIGH);
		
		changeStatus(UA_STATE_IDLE);
	}

	/** Callback function called when the invite expires */
	public void onCallTimeout(Call call) {
		printLog("onCallTimeout()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("NOT FOUND/TIMEOUT", LogLevel.HIGH);
		changeStatus(UA_STATE_IDLE);
		if (call == call_transfer) {
			int code = 408;
			String reason = "Request Timeout";
			this.call.notify(code, reason);
			call_transfer = null;
		}
	}

	// ****************** ExtendedCall callback functions ******************

	/**
	 * Callback function called when arriving a new REFER method (transfer
	 * request)
	 */
	public void onCallTransfer(ExtendedCall call, NameAddress refer_to,
			NameAddress refered_by, Message refer) {
		printLog("onCallTransfer()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("Transfer to " + refer_to.toString(), LogLevel.HIGH);
		call.acceptTransfer();
		call_transfer = new ExtendedCall(sip_provider, user_profile.from_url,
				user_profile.contact_url, this);
		call_transfer.call(refer_to.toString(), local_session, null); 		// modified by mandrajg
	}

	/** Callback function called when a call transfer is accepted. */
	public void onCallTransferAccepted(ExtendedCall call, Message resp) {
		printLog("onCallTransferAccepted()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("Transfer accepted", LogLevel.HIGH);
	}

	/** Callback function called when a call transfer is refused. */
	public void onCallTransferRefused(ExtendedCall call, String reason,
			Message resp) {
		printLog("onCallTransferRefused()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("Transfer refused", LogLevel.HIGH);
	}

	/** Callback function called when a call transfer is successfully completed */
	public void onCallTransferSuccess(ExtendedCall call, Message notify) {
		printLog("onCallTransferSuccess()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("Transfer successed", LogLevel.HIGH);
		call.hangup();
	}

	/**
	 * Callback function called when a call transfer is NOT sucessfully
	 * completed
	 */
	public void onCallTransferFailure(ExtendedCall call, String reason,
			Message notify) {
		printLog("onCallTransferFailure()", LogLevel.LOW);
		if (call != this.call) {
			printLog("NOT the current call", LogLevel.LOW);
			return;
		}
		printLog("Transfer failed", LogLevel.HIGH);
	}

	// ************************* Schedule events ***********************

	/** Schedules a re-inviting event after <i>delay_time</i> secs. */
	void reInvite(final String contact_url, final int delay_time) {
		SessionDescriptor sdp = new SessionDescriptor(local_session);
		sdp.IncrementOLine();
		final SessionDescriptor new_sdp;
		if (statusIs(UserAgent.UA_STATE_INCALL)) { // modified
			new_sdp = new SessionDescriptor(
					sdp.getOrigin(), sdp.getSessionName(), new ConnectionField(
							"IP4", "0.0.0.0"), new TimeField());
		} else {
			new_sdp = new SessionDescriptor(
					sdp.getOrigin(), sdp.getSessionName(), new ConnectionField(
							"IP4", IpAddress.localIpAddress), new TimeField());
		}
		new_sdp.addMediaDescriptors(sdp.getMediaDescriptors());
		local_session = sdp.toString();
		(new Thread() {
			public void run() {
				runReInvite(contact_url, new_sdp.toString(), delay_time);
			}
		}).start();
	}

	/** Re-invite. */
	private void runReInvite(String contact, String body, int delay_time) {
		try {
			if (delay_time > 0)
				Thread.sleep(delay_time * 1000);
		} catch (Exception e) {
			e.printStackTrace();
		}
			printLog("RE-INVITING/MODIFYING");
			if (call != null && call.isOnCall()) {
				printLog("REFER/TRANSFER");
				call.modify(contact, body);
			}
	}

	/** Schedules a call-transfer event after <i>delay_time</i> secs. */
	void callTransfer(final String transfer_to, final int delay_time) {
		// in case of incomplete url (e.g. only 'user' is present), try to
		// complete it
		final String target_url;
		if (transfer_to.indexOf("@") < 0)
			target_url = transfer_to + "@" + realm; // modified
		else
			target_url = transfer_to;
		(new Thread() {
			public void run() {
				runCallTransfer(target_url, delay_time);
			}
		}).start();
	}

	/** Call-transfer. */
	private void runCallTransfer(String transfer_to, int delay_time) {
		try {
			if (delay_time > 0)
				Thread.sleep(delay_time * 1000);
		} catch (Exception e) {
			e.printStackTrace();
		}
			if (call != null && call.isOnCall()) {
				printLog("REFER/TRANSFER");
				call.transfer(transfer_to);
			}
	}

	/** Schedules an automatic answer event after <i>delay_time</i> secs. */
	void automaticAccept(final int delay_time) {
		(new Thread() {
			public void run() {
				runAutomaticAccept(delay_time);
			}
		}).start();
	}

	/** Automatic answer. */
	private void runAutomaticAccept(int delay_time) {
		try {
			if (delay_time > 0)
				Thread.sleep(delay_time * 1000);
		} catch (Exception e) {
			e.printStackTrace();
		}
			if (call != null) {
				printLog("AUTOMATIC-ANSWER");
				accept();
			}
	}

	/** Schedules an automatic hangup event after <i>delay_time</i> secs. */
	void automaticHangup(final int delay_time) {
		(new Thread() {
			public void run() {
				runAutomaticHangup(delay_time);
			}
		}).start();
	}

	/** Automatic hangup. */
	private void runAutomaticHangup(int delay_time) {
		try {
			if (delay_time > 0)
				Thread.sleep(delay_time * 1000);
		} catch (Exception e) {
			e.printStackTrace();
		}
			if (call != null && call.isOnCall()) {
				printLog("AUTOMATIC-HANGUP");
				hangup();
			}

	}

	// ****************************** Logs *****************************

	/** Adds a new string to the default Log */
	void printLog(String str) {
		printLog(str, LogLevel.HIGH);
	}

	/** Adds a new string to the default Log */
	void printLog(String str, int level) {
		if (logger != null)
			logger.info("UA: " + str); // FIXME - must use level argument
	}

	/** Adds the Exception message to the default Log */
	void printException(Exception e, int level) {
		if (Sipdroid.release) return;
		if (logger != null)
			logger.log(Level.SEVERE, "exception", e);
		else
			e.printStackTrace();
			
	}

}
