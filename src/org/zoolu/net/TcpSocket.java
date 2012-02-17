/*
 * Copyright (C) 2005 Luca Veltri - University of Parma - Italy
 * Copyright (C) 2009 The Sipdroid Open Source Project
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

package org.zoolu.net;

import java.net.InetSocketAddress;
import java.net.Socket; // import java.net.InetAddress;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.logging.Logger;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import org.opentelecoms.util.crypto.AppendingTrustManager;

/* import org.bouncycastle.crypto.tls.AlwaysValidVerifyer;
import org.bouncycastle.crypto.tls.CertificateVerifyer;
import org.bouncycastle.crypto.tls.TlsProtocolHandler; */

/**
 * TcpSocket provides a uniform interface to TCP transport protocol, regardless
 * J2SE or J2ME is used.
 */
public class TcpSocket {
	
	static SecureRandom secureRandom = new SecureRandom();
	
	private Logger logger = Logger.getLogger(getClass().getCanonicalName());
	
	// For testing with BouncyCastle light TLS implementation
	// TlsProtocolHandler tlsHandler = null;
	

	
	/** Socket */
	Socket socket;

	private boolean useTls;

	/** Creates a new TcpSocket */
	TcpSocket() {
		socket = null;
	}

	/** Creates a new TcpSocket */
	TcpSocket(Socket sock) {
		socket = sock;
	}

	static HashMap<IpAddress,String> inProgress = new HashMap<IpAddress,String>();
	static void startProgress(IpAddress addr) throws IOException {
		synchronized (inProgress) {
			String s = inProgress.get(addr);
			if(s != null)
				throw new java.io.IOException("TcpSocket/already inProgress");
			inProgress.put(addr, "");
		}
	}
	
	static void finishProgress(IpAddress addr) {
		synchronized (inProgress) {
			inProgress.remove(addr);
		}
	}
	
	SSLContext sslContext = null;
	SSLContext getSSLContext() throws NoSuchAlgorithmException, KeyStoreException, KeyManagementException {
		synchronized(this) {
			if(sslContext == null) {
			    TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
			    tmf.init((KeyStore)null);
			    TrustManager[] tm = tmf.getTrustManagers();
			    TrustManager[] tm2 = { new AppendingTrustManager(
			    		(X509TrustManager)tm[0], customKeyStore) };
			    //TrustManager[] tm = { new ShoddyTrustManager() };
				//sslContext = SSLContext.getInstance("TLSv1");
				sslContext = SSLContext.getInstance("TLS");
				//sslContext = SSLContext.getInstance("TLS");
				//logger.info("Using default trust manager");
				//sslContext.init(null, null, secureRandom);
				sslContext.init(null, tm2, secureRandom);
			}
		}
		return sslContext;
	}
	
	/** Creates a new UdpSocket */
	public TcpSocket(IpAddress ipaddr, int port, boolean _useTls) throws java.io.IOException {
//		socket = new Socket(ipaddr.getInetAddress(), port); modified
		
		startProgress(ipaddr);
		
		useTls = _useTls;
		
		if(!useTls)
			socket = new Socket();
		else {
			/* FIXME: BIG WARNING
			 * 
			 * This is NOT a full TLS implementation for SIP
			 * 
			 * It ONLY verifies that we are connecting to the named server
			 * 
			 * Proper SIP TLS compliance requires that we verify the server
			 * certificate is valid for the SIP domain of each SIP message
			 * we send to the server.
			 * 
			 * However, this initial implementation is
			 * - still more secure than ordinary TCP or UDP based SIP, without
			 *   bringing in any new vulnerabilities
			 * - suitable for connecting to an outbound proxy that is
			 *   explicitly configured in the UA (user implicitly trusts the proxy)
			 * - serves to prevent SIP-aware NAT devices from mangling the SIP
			 *   headers and SDP body
			 * - serves to prevent a man-in-the-middle from snooping on SIP traffic
			 *   (e.g. a mobile phone company that tries to detect and punish users who
			 *   use VoIP on mobile broadband)
			 */
			
			try {
				SSLSocketFactory socketFactory = getSSLContext().getSocketFactory();
				socket = socketFactory.createSocket();
			} catch (Exception e) {
				e.printStackTrace();
				logger.warning("IOException/failure in the SSL init: " + e.getClass().getCanonicalName() + ": " + e.getMessage());
				throw new IOException(e.getClass().getCanonicalName() + ": " + e.getMessage());
			}
		}
		try {
			socket.connect(new InetSocketAddress(ipaddr.toString(), port),
				Thread.currentThread().getName().equals("main")?1000:10000);
		} catch (java.io.IOException e) {
			finishProgress(ipaddr);
			logger.warning("IOException/failure in the connect method: " + e.getMessage());
			throw e;
		}
		if(useTls) {
			logger.info("Checking SSL session validity");
			SSLSocket _socket = (SSLSocket)socket;
			SSLSession session = _socket.getSession();
			if(session.isValid()) {
				logger.info("Secure connection established");
			} else {
				finishProgress(ipaddr);
				logger.warning("Connection NOT secure");
				throw new IOException("SSLSession NOT valid/secure");
			}
			
			// For testing with BouncyCastle light TLS implementation
			/* tlsHandler = new TlsProtocolHandler(socket.getInputStream(), socket.getOutputStream());
			CertificateVerifyer cv = new AlwaysValidVerifyer();
			tlsHandler.connect(cv); */
		}
		finishProgress(ipaddr);
	}

	/** Closes this socket. */
	public void close() throws java.io.IOException {
		socket.close();
	}

	/** Gets the address to which the socket is connected. */
	public IpAddress getAddress() {
		return new IpAddress(socket.getInetAddress());
	}

	/** Gets an input stream for this socket. */
	public InputStream getInputStream() throws java.io.IOException {
		// For testing with BouncyCastle light TLS implementation
		/* if(tlsHandler != null)
			return tlsHandler.getInputStream(); */
		return socket.getInputStream();
	}

	/** Gets the local address to which the socket is bound. */
	public IpAddress getLocalAddress() {
		return new IpAddress(socket.getLocalAddress());
	}

	/** Gets the local port to which this socket is bound. */
	public int getLocalPort() {
		return socket.getLocalPort();
	}

	/** Gets an output stream for this socket. */
	public OutputStream getOutputStream() throws java.io.IOException {
		// For testing with BouncyCastle light TLS implementation
		/* if(tlsHandler != null)
			return tlsHandler.getOutputStream(); */
		return socket.getOutputStream();
	}

	/** Gets the remote port to which this socket is connected. */
	public int getPort() {
		return socket.getPort();
	}

	/** Gets the socket timeout. */
	public int getSoTimeout() throws java.net.SocketException {
		return socket.getSoTimeout();
	}

	/** Enables/disables the socket timeou, in milliseconds. */
	public void setSoTimeout(int timeout) throws java.net.SocketException {
		socket.setSoTimeout(timeout);
	}

	/** Converts this object to a String. */
	public String toString() {
		return socket.toString();
	}

	public boolean isTls() {
		return useTls;
	}

	static KeyStore customKeyStore = null;
	public static void setCustomKeyStore(KeyStore trusted) {
		customKeyStore = trusted;
		
	}

}
