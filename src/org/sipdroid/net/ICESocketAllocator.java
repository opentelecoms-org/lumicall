package org.sipdroid.net;

import java.net.DatagramSocket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;

import org.ice4j.ice.Agent;
import org.ice4j.ice.Candidate;
import org.ice4j.ice.CandidatePair;
import org.ice4j.ice.Component;
import org.ice4j.ice.IceMediaStream;

public class ICESocketAllocator implements SocketAllocator {
	
	private static final Logger logger
    = Logger.getLogger(ICESocketAllocator.class.getName());
	
	Map<Integer, DatagramSocket> sockets = null;
	
	Agent iceAgent;
	
	public ICESocketAllocator(Agent agent) {
		this.iceAgent = agent;
		handleICECandidates(agent);
	}
	
	public void free() {
		for(DatagramSocket socket : sockets.values()) {
			//socket.close();    // FIXME - what should we do here?
		}
		//iceAgent.free();
	}

	@Override
	public DatagramSocket allocateSocket(int port) throws SocketException,
			UnknownHostException {
		// TODO Auto-generated method stub
		if(sockets == null)
			throw new SocketException("not initialised");
		DatagramSocket socket = sockets.get(new Integer(port));
		if(socket == null)
			throw new SocketException("socket not setup by ICE on port: " + port);
		return socket;
	}
	
	/* Takes the candidate pairs selected by ICE
	 * and makes them available for the application to get
	 * them with calls to allocateSocket()
	 */
	protected void handleICECandidates(Agent agent) {
		sockets = new HashMap<Integer, DatagramSocket>();
		for(IceMediaStream ims : agent.getStreams()) {
			logger.info("handling stream: " + ims.getName());
			for(Component component : ims.getComponents()) {
				CandidatePair selectedPair = component.getSelectedPair();

                if (selectedPair != null) {
                    DatagramSocket streamConnectorSocket
                        = selectedPair.getDatagramSocket();

                    if (streamConnectorSocket != null) {
                    	//Integer port = new Integer(streamConnectorSocket.getPort());
                    	//Candidate defaultCandidate = component.getDefaultCandidate();
                    	Candidate candidate = selectedPair.getLocalCandidate();   
                    	if(candidate != null) {
                    		//Integer port = new Integer(component.getDefaultCandidate().getTransportAddress().getPort());
                    		Integer port = new Integer(candidate.getTransportAddress().getPort());
                    		//Integer port = new Integer(component.getComponentID() + 20999);
                    		logger.info("found stream: " + ims.getName() + ", component: " + component.getComponentID() + ", port: " + port);
                    		sockets.put(port, streamConnectorSocket);
                    	} else {
                    		throw new RuntimeException("no default candidate for stream: " + ims.getName() + ", component: " + component.getComponentID());
                    	}
                    } else {
                    	throw new RuntimeException("no socket for stream: " + ims.getName() + ", component: " + component.getComponentID());
                    }
                } else {
                	throw new RuntimeException("no selected pair for stream: " + ims.getName() + ", component: " + component.getComponentID());
                }
			}
		}
	}

}
