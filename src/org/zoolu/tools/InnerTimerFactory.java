package org.zoolu.tools;

public interface InnerTimerFactory {
	
	public InnerTimer createInnerTimer(long timeout, InnerTimerListener listener);

}
