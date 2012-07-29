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

package org.zoolu.net;

/**
 * A SocketAddress is a pair { address, port }.
 */
public class SocketAddress {
	/** The InetAddress */
	IpAddress ipaddr;

	/** The port */
	int port;

	/** Creates a SocketAddress. */
	public SocketAddress(IpAddress ipaddr, int port) {
		init(ipaddr, port);
	}

	/** Creates a SocketAddress. */
	public SocketAddress(String addr, int port) {
		init(new IpAddress(addr), port);
	}

	/** Creates a SocketAddress. */
	public SocketAddress(String soaddr) {
		String addr = null;
		int port = -1;
		int colon = soaddr.indexOf(':');
		if (colon < 0)
			addr = soaddr;
		else {
			addr = soaddr.substring(0, colon);
			try {
				port = Integer.parseInt(soaddr.substring(colon + 1));
			} catch (Exception e) {
			}
		}
		init(new IpAddress(addr), port);
	}

	/** Creates a SocketAddress. */
	public SocketAddress(SocketAddress soaddr) {
		init(soaddr.ipaddr, soaddr.port);
	}

	/** Inits the SocketAddress. */
	private void init(IpAddress ipaddr, int port) {
		this.ipaddr = ipaddr;
		this.port = port;
	}

	/** Gets the host address. */
	public IpAddress getAddress() {
		return ipaddr;
	}

	/** Gets the port. */
	public int getPort() {
		return port;
	}

	/** Makes a copy. */
	public Object clone() {
		return new SocketAddress(this);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((ipaddr == null) ? 0 : ipaddr.hashCode());
		result = prime * result + port;
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		SocketAddress other = (SocketAddress) obj;
		if (ipaddr == null) {
			if (other.ipaddr != null)
				return false;
		} else if (!ipaddr.equals(other.ipaddr))
			return false;
		if (port != other.port)
			return false;
		return true;
	}

	/** Gets a String representation of the Object. */
	public String toString() {
		return (ipaddr.toString() + ":" + port);
	}

}
