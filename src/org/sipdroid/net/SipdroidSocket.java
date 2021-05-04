/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
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

package org.sipdroid.net;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketOptions;
import java.net.UnknownHostException;
import java.util.logging.Logger;

import org.sipdroid.net.impl.OSNetworkSystem;
import org.sipdroid.net.impl.PlainDatagramSocketImpl;

public class SipdroidSocket extends DatagramSocket {
	
	private static final Logger logger
    	= Logger.getLogger(SipdroidSocket.class.getName());

	PlainDatagramSocketImpl impl = null;
	public static boolean loaded = false;
	
	private static boolean useJNIImpl = false;
	
	public static void enableJNIImpl(boolean enable) {
		if(loaded)
			useJNIImpl = enable;
	}
	
	public SipdroidSocket(int port) throws SocketException, UnknownHostException {
		super(!useJNIImpl?port:0);
		if (useJNIImpl) {
			impl = new PlainDatagramSocketImpl();
			impl.create();
			impl.bind(port,InetAddress.getByName("0.0.0.0"));
		}
	}
	
	public SipdroidSocket(int port, InetAddress laddr) throws SocketException {
		super(!useJNIImpl?port:0, !useJNIImpl?laddr:null);
		if (useJNIImpl) {
			impl = new PlainDatagramSocketImpl();
			impl.create();
			impl.bind(port, laddr);
		}
	}

	public SipdroidSocket(SocketAddress bindaddr) throws SocketException {
		super(!useJNIImpl?bindaddr:null);
		InetSocketAddress _bindaddr = (InetSocketAddress)bindaddr;
		if (useJNIImpl) {
			impl = new PlainDatagramSocketImpl();
			impl.create();
			impl.bind(_bindaddr.getPort(),_bindaddr.getAddress());
		}
	}

	public SipdroidSocket() throws SocketException {
		super(0);
		if (useJNIImpl) {
			impl = new PlainDatagramSocketImpl();
			impl.create();
		}
	}
	
	@Override
	public void bind(SocketAddress sa) throws SocketException {
		if(impl != null) {
			if(sa.getClass().isInstance(InetSocketAddress.class)) {
				InetSocketAddress _sa = (InetSocketAddress)sa;
				impl.bind(_sa.getPort(), _sa.getAddress());
			} else
				throw new SocketException("sa must be an instance of InetSocketAddress");
		} else
			super.bind(sa);
	}
	
	public void close() {
		super.close();
		if (impl != null) impl.close();
	}
	
	public void setSoTimeout(int val) throws SocketException {
		if (impl != null) impl.setOption(SocketOptions.SO_TIMEOUT, val);
		else super.setSoTimeout(val);
	}
	
	public void receive(DatagramPacket pack) throws IOException {
		if (impl != null) impl.receive(pack);
		else super.receive(pack);
	}
	
	public void send(DatagramPacket pack) throws IOException {
		if (impl != null) impl.send(pack);
		else super.send(pack);
	}
	
	public boolean isConnected() {
		if (impl != null) return true;
		else return super.isConnected();
	}
	
	public void disconnect() {
		if (impl == null) super.disconnect();
	}
	
	public void connect(InetAddress addr,int port) {
		if (impl == null) super.connect(addr,port);
	}
	
	public boolean isBound() {
		if(impl != null)
			throw new RuntimeException("Method not implemented");
		return super.isBound();
	}

	static {
			try {
		        System.loadLibrary("OSNetworkSystem");
		        OSNetworkSystem.getOSNetworkSystem().oneTimeInitialization(true);
		        SipdroidSocket.loaded = true;
		        SipdroidSocket.useJNIImpl = true;
			} catch (Throwable e) {
				logger.severe("JNI/Exception loading/initializing OSNetworkSystem: " + 
						e.getClass().getCanonicalName()	+
						": " + e.getMessage());
			}
	}
}
