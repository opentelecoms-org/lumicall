package org.sipdroid.net;

import java.net.DatagramSocket;
import java.net.SocketException;
import java.net.UnknownHostException;

public interface SocketAllocator {
	
	public void free();
	
	public DatagramSocket allocateSocket(int port) throws SocketException, UnknownHostException;

}
