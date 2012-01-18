package org.sipdroid.sipua;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import java.net.BindException;
import java.net.DatagramSocket;

import org.ice4j.Transport;
import org.ice4j.ice.Agent;
import org.ice4j.ice.CandidatePair;
import org.ice4j.ice.Component;
import org.ice4j.ice.IceMediaStream;
import org.ice4j.ice.IceProcessingState;
import org.lumicall.android.R;

public class MediaStream {
	
	final static int PORT_RANGE = 6;
	
	String name;
	int desiredPort;
	int rtpPort;
	int rtcpPort;
	
	int remoteRtpPort = -1;
	String remoteRtpAddress = null;
	int remoteRtcpPort = -1;
	String remoteRtcpAddress = null;
	
	Agent iceAgent;
	IceMediaStream iceStream;

	private DatagramSocket rtpSocket;
	
	public MediaStream(Agent iceAgent, String name, int desiredPort) throws BindException, IllegalArgumentException, IOException {
		this.name = name;
		this.desiredPort = desiredPort;
		int desiredRtcpPort = desiredPort + 1;
		
		this.iceAgent = iceAgent;
		
		iceStream = iceAgent.createMediaStream(name);
        //rtp
        Component rtpComponent = iceAgent.createComponent(
                iceStream, Transport.UDP, desiredPort, desiredPort, desiredPort + PORT_RANGE);
        rtpPort = rtpComponent.getLocalCandidates().get(0).getTransportAddress().getPort();
        
        //rtcpComp
        int maxRtcpPort = desiredRtcpPort + PORT_RANGE;
        desiredRtcpPort = rtpPort + 1;
        Component rtcpComponent = iceAgent.createComponent(
                iceStream, Transport.UDP, desiredRtcpPort, desiredRtcpPort, maxRtcpPort);
        rtcpPort = rtcpComponent.getLocalCandidates().get(0).getTransportAddress().getPort();
        
        // FIXME ICE testing
        //iceAgent.addStateChangeListener(new IceProcessingListener());
	}
	
	public String getName() {
		return name;
	}
	
	public int getDesiredPort() {
		return desiredPort;
	}

	public int getRtpPort() {
		return rtpPort;
	}

	public int getRtcpPort() {
		return rtcpPort;
	}

	public IceMediaStream getIceMediaStream() {
		// TODO Auto-generated method stub
		return iceStream;
	}
	
	public int getRemoteRtpPort() {
		return remoteRtpPort;
	}

	public String getRemoteRtpAddress() {
		return remoteRtpAddress;
	}

	public int getRemoteRtcpPort() {
		return remoteRtcpPort;
	}

	public String getRemoteRtcpAddress() {
		return remoteRtcpAddress;
	}

	protected int getPortForComponent(int componentId) {
		return iceStream.getComponent(componentId).getSelectedPair().getRemoteCandidate().getTransportAddress().getPort();
	}
	
	protected String getAddressForComponent(int componentId) {
		return iceStream.getComponent(componentId).getSelectedPair().getRemoteCandidate().getTransportAddress().getAddress().getHostAddress();
	}
	
	protected DatagramSocket getDatagramSocketForComponent(int componentId) {
		return iceStream.getComponent(componentId).getSelectedPair().getLocalCandidate().
                getDatagramSocket();
	}
	
	protected void handleCompletion() {
		remoteRtpPort = getPortForComponent(Component.RTP);
		remoteRtpAddress = getAddressForComponent(Component.RTP);
		remoteRtcpPort = getPortForComponent(Component.RTCP);
		remoteRtcpAddress = getAddressForComponent(Component.RTCP);
		
		rtpSocket = getDatagramSocketForComponent(Component.RTP);
	}

	public class IceProcessingListener implements PropertyChangeListener {
		
		@Override
		public void propertyChange(PropertyChangeEvent event) {
			IceProcessingState iceProcessingState = (IceProcessingState)event.getNewValue();
			
			switch (iceProcessingState) {
			case COMPLETED:
				MediaStream.this.handleCompletion();
				break;
			default:
				return;
			}
			
		}
		
	}

	public DatagramSocket getRTPSocket() {
		// TODO Auto-generated method stub
		return rtpSocket;
	}

	

}
