package org.sipdroid.net;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import org.ice4j.socket.DatagramSocketFactory;

public class SipdroidSocketFactory implements DatagramSocketFactory {

	@Override
	public DatagramSocket createDatagramSocket() throws SocketException {
		return new SipdroidSocket();
	}

	@Override
	public DatagramSocket createDatagramSocket(int port) throws SocketException, UnknownHostException {
		return new SipdroidSocket(port);
	}

	@Override
	public DatagramSocket createDatagramSocket(int port, InetAddress laddr) throws SocketException {
		return new SipdroidSocket(port, laddr);
	}

	@Override
	public DatagramSocket createDatagramSocket(SocketAddress bindaddr) throws SocketException {
		// TODO Auto-generated method stub
		return new SipdroidSocket(bindaddr);
	}

}
