package org.sipdroid.net;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import org.ice4j.socket.DatagramSocketFactory;

public class SipdroidSocketFactory implements DatagramSocketFactory {

	@Override
	public DatagramSocket createUnboundDatagramSocket() throws SocketException {
		return new SipdroidSocket();
	}

}
