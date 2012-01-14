package org.sipdroid.net;

import java.net.DatagramSocket;
import java.net.SocketException;
import java.net.UnknownHostException;

public class SipdroidSocketAllocator implements SocketAllocator {

	public void free() {
		
	}
	
	@Override
	public DatagramSocket allocateSocket(int port) throws SocketException, UnknownHostException {
		// TODO Auto-generated method stub
		return new SipdroidSocket(port);
	}

}
