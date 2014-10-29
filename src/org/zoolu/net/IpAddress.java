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

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.Vector;
import java.util.logging.Logger;

/**
 * IpAddress is an IP address.
 */
public class IpAddress {
	
	static Logger logger = Logger.getLogger(IpAddress.class.getCanonicalName());

	/** The host address/name */
	String address;

	/** The InetAddress */
	InetAddress inet_address;
	
	/** Local IP address */
	public static String localIpAddress = "127.0.0.1";
	
	// ********************* Protected *********************

	/** Creates an IpAddress */
	public IpAddress(InetAddress iaddress) {
		init(null, iaddress);
	}

	/** Inits the IpAddress */
	private void init(String address, InetAddress iaddress) {
		this.address = address;
		this.inet_address = iaddress;
	}

	/** Gets the InetAddress */
	InetAddress getInetAddress() {
		if (inet_address == null)
			try {
				inet_address = InetAddress.getByName(address);
			} catch (java.net.UnknownHostException e) {
				inet_address = null;
			}
		return inet_address;
	}

	// ********************** Public ***********************

	/** Creates an IpAddress */
	public IpAddress(String address) {
		init(address, null);
	}

	/** Creates an IpAddress */
	public IpAddress(IpAddress ipaddr) {
		init(ipaddr.address, ipaddr.inet_address);
	}

	/** Gets the host address */
	/*
	 * public String getAddress() { if (address==null)
	 * address=inet_address.getHostAddress(); return address; }
	 */

	/** Makes a copy */
	public Object clone() {
		return new IpAddress(this);
	}

	/** Wthether it is equal to Object <i>obj</i> */
	@Override
	public boolean equals(Object obj) {
		// FIXME - toString() does a DNS lookup, causing this method
		// to run slowly
		try {
			IpAddress ipaddr = (IpAddress) obj;
			if (!toString().equals(ipaddr.toString()))
				return false;
			return true;
		} catch (Exception e) {
			return false;
		}
	}
	
	@Override
	public int hashCode() {
		// FIXME - toString() does a DNS lookup, causing this method
		// to run slowly
		String s = toString();
		if(s == null)
			throw new RuntimeException("Invalid IpAddress/can't resolve toString()");
		return s.hashCode();
	}

	/** Gets a String representation of the Object */
	public String toString() {
		// FIXME - should not call getHostAddress() in toString()!!!
		if (address == null && inet_address != null)
			address = inet_address.getHostAddress();
		return address;
	}

	// *********************** Static ***********************

	/** Gets the IpAddress for a given fully-qualified host name. */
	public static IpAddress getByName(String host_addr)
			throws java.net.UnknownHostException {
		InetAddress iaddr = InetAddress.getByName(host_addr);
		return new IpAddress(iaddr);
	}
	
	/** Sets the local IP address into the variable <i>localIpAddress</i> */
	public static void setLocalIpAddress() {
		localIpAddress = "127.0.0.1";
		
		Vector<InetAddress> preferredResults = new Vector<InetAddress>();
		Vector<InetAddress> extraResults = new Vector<InetAddress>();
		
		String _prefer_v4 = System.getProperty("java.net.preferIPv4Stack");
		boolean prefer_v4 = (_prefer_v4 != null && Boolean.valueOf(_prefer_v4));
		logger.fine("java.net.preferIPv4Stack: " + _prefer_v4 + ", prefer IPv4: " + prefer_v4);

		try {
			for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
				NetworkInterface intf = en.nextElement();

				for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
					InetAddress inetAddress = enumIpAddr.nextElement();

					if (!inetAddress.isLoopbackAddress() && !inetAddress.isLinkLocalAddress()) {
						if(prefer_v4 && !(inetAddress instanceof Inet4Address)) {
							extraResults.add(inetAddress);
						} else {
							preferredResults.add(inetAddress);
						}
						
					}					
				}
			}
		} catch (SocketException ex) {
			// do nothing
		}
		InetAddress inetAddress = null;
		if(preferredResults.size() > 0) {
			inetAddress = preferredResults.get(0);
		} else if(extraResults.size() > 0) {
			inetAddress = extraResults.get(0);
		}
		if(inetAddress != null) {
			localIpAddress = inetAddress.getHostAddress().toString();
		}
		logger.fine("Selected localIpAddress: " + localIpAddress);
	}
}
