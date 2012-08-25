/*
 * Copyright (C) 2005 Luca Veltri - University of Parma - Italy
 * 
 * This file is part of MjSip (http://www.mjsip.org)
 * 
 * MjSip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * MjSip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MjSip; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author(s):
 * Luca Veltri (luca.veltri@unipr.it)
 */

package org.zoolu.sip.provider;

import org.zoolu.net.*;
import org.zoolu.sip.message.Message;
import org.zoolu.tools.Timer;
import org.zoolu.tools.TimerListener;

import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

/**
 * TcpTransport provides a TCP trasport service for SIP.
 */
class TcpTransport implements ConnectedTransport, TcpConnectionListener {
	
	private static Logger log =
	        Logger.getLogger(TcpTransport.class.getName());
	
	/** TCP protocol type */
	public static final String PROTO_TCP = "tcp";
	public static final String PROTO_TLS = "tls";
	private String protocol = null;

	/** TCP connection */
	TcpConnection tcp_conn;

	/** TCP connection */
	ConnectionIdentifier connection_id;

	/** The last time that has been used (in milliseconds) */
	long last_time;

	/** the current received text. */
	String text;

	/** Transport listener */
	TransportListener listener;
	
	/** Timer for sending Outbound (RFC 5626) keep-alive */
	Timer keepAliveTimer = null;
	KeepAliveSender keepAlive = null;
	/** Default keep-alive interval for
	 * battery-powered devices as per RFC 5626 */
	long keepAliveInterval = TimeUnit.SECONDS.toMillis(840);
	long keepAlivePongTimeout = TimeUnit.SECONDS.toMillis(10);
	private final static String CRLF = "" + '\r' + '\n';
	private final static String PING = CRLF + CRLF;
	private final static String PONG = CRLF;
	
	private class KeepAliveSender implements TimerListener {
		
		volatile boolean pingSent = false;
		
		protected void setTimer(long interval) {
			keepAliveTimer = new Timer(interval, this);
			keepAliveTimer.start();
		}

		@Override
		public void onTimeout(Timer t) {
			Exception tcp_failed = null;
			
			if(!pingSent) {
				log.info("Doing TCP outbound Keep-Alive ping");
				if(tcp_conn != null) {
					try {
						synchronized(tcp_conn) {
							tcp_conn.send(PING.getBytes());
						}
					} catch (IOException e) {
						log.severe("IOException while trying to send ping, maybe connection is down?");
						tcp_failed = e;
					}
					setTimer(keepAlivePongTimeout);
					pingSent = true;
				} else {
					log.severe("tcp_conn is null, can't send any keep-alive");
					tcp_failed = new IOException("tcp_conn is null, can't send keep-alive");
				}
			} else {
				log.info("No response to keep-alive ping: killing connection");
				tcp_failed = new IOException("no response to keep-alive Ping");
			}
			if(tcp_failed != null) {
				log.severe("tcp_failed: " + tcp_failed.getMessage());
				pingSent = false;
				onConnectionTerminated(tcp_conn, tcp_failed);
				tcp_failed = null;
			}
		}
		
		public void onPong() {
			if(pingSent) {
				keepAliveTimer.halt();
				log.info("Got response to keep-alive ping: pong");
				pingSent = false;
				setTimer(keepAliveInterval);
			} else {
				log.info("Got unexpected pong from peer");
			}	
		}

		public void onDataReceived() {
			keepAliveTimer.halt();
			if(pingSent) {
				log.info("Got data from peer while waiting for keep-alive pong");
				pingSent = false;
			}
			setTimer(keepAliveInterval);
		}
	}
	
	private static String getProtocolString(boolean forTls) {
		return forTls ? PROTO_TLS : PROTO_TCP;
	}
	
	/** Creates a new TcpTransport */
	public TcpTransport(IpAddress remote_ipaddr, int remote_port,
			TransportListener listener, boolean useTls) throws IOException {
		protocol = getProtocolString(useTls);
		this.listener = listener;
		TcpSocket socket = new TcpSocket(remote_ipaddr, remote_port, useTls);
		tcp_conn = new TcpConnection(socket, this);
		connection_id = new ConnectionIdentifier(this);
		last_time = System.currentTimeMillis();
		text = "";
		
		keepAlive = new KeepAliveSender();
		keepAliveTimer = new Timer(keepAliveInterval, keepAlive);
		keepAliveTimer.start();
	}

	/** Costructs a new TcpTransport */
	public TcpTransport(TcpSocket socket, TransportListener listener) {
		this.listener = listener;
		tcp_conn = new TcpConnection(socket, this);
		protocol = getProtocolString(socket.isTls());
		connection_id = null;
		last_time = System.currentTimeMillis();
		text = "";
	}

	/** Gets protocol type */
	public String getProtocol() {
		return protocol;
	}

	/** Gets the remote IpAddress */
	public IpAddress getRemoteAddress() {
		if (tcp_conn != null)
			return tcp_conn.getRemoteAddress();
		else
			return null;
	}

	/** Gets the remote port */
	public int getRemotePort() {
		if (tcp_conn != null)
			return tcp_conn.getRemotePort();
		else
			return 0;
	}

	/** Gets the last time the Connection has been used (in millisconds) */
	public long getLastTimeMillis() {
		return last_time;
	}

	/**
	 * Sends a Message through the connection. Parameters <i>dest_addr</i>/<i>dest_addr</i>
	 * are not used, and the message is addressed to the connection remote peer.
	 * <p>
	 * Better use sendMessage(Message msg) method instead.
	 */
	public void sendMessage(Message msg, IpAddress dest_ipaddr, int dest_port)
			throws IOException {
		sendMessage(msg);
	}

	/** Sends a Message */
	public void sendMessage(Message msg) throws IOException {
		if (tcp_conn != null) {
			last_time = System.currentTimeMillis();
			byte[] data = msg.toString().getBytes();
			synchronized(tcp_conn) {
				tcp_conn.send(data);
			}
		} else {
			log.severe("attempt to send message on a TcpTransport that is not available");
			throw new IOException("no tcp_conn available");
		}
	}

	/** Stops running */
	public void halt() {
		if (tcp_conn != null)
			tcp_conn.halt();
	}

	/** Gets a String representation of the Object */
	public String toString() {
		if (tcp_conn != null)
			return tcp_conn.toString();
		else
			return null;
	}

	// ************************* Callback methods *************************

	/** When new data is received through the TcpConnection. */
	public void onReceivedData(TcpConnection tcp_conn, byte[] data, int len) {
		last_time = System.currentTimeMillis();

		text += new String(data, 0, len);
		SipParser par = new SipParser(text);
		if(keepAlive != null) {
			if(text.startsWith(PONG)) {
				/* As keepAlive != null, we presume that we are the client
				 * and that this can only be a PONG and not a PING.
				 * FIXME: if this code needs to act as a server-side UA,
				 * it needs to distinguish received PINGs from PONGs,
				 * not just assume CRLF is always a PONG
				 */
				keepAlive.onPong();
				text = text.substring(PONG.length());
			} else {
				keepAlive.onDataReceived();
			}
		}
		Message msg = par.getSipMessage();
		while (msg != null) { // System.out.println("DEBUG: message len:
			// "+msg.getLength());
			msg.setRemoteAddress(tcp_conn.getRemoteAddress().toString());
			msg.setRemotePort(tcp_conn.getRemotePort());
			msg.setTransport(getProtocol());
			msg.setConnectionId(connection_id);
			if (listener != null)
				listener.onReceivedMessage(this, msg);

			text = par.getRemainingString();
			// System.out.println("DEBUG: text left: "+text.length());
			par = new SipParser(text);
			msg = par.getSipMessage();
		}
	}

	/** When TcpConnection terminates. */
	public void onConnectionTerminated(TcpConnection tcp_conn, Exception error) {
		if (listener != null)
			listener.onTransportTerminated(this, error);
		TcpSocket socket = tcp_conn.getSocket();
		if (socket != null)
			try {
				socket.close();
			} catch (Exception e) {
			}
		this.tcp_conn = null;
		this.listener = null;
	}

}
