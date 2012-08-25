package org.zoolu.tools;

public class BasicTimerFactory implements InnerTimerFactory {

	@Override
	public InnerTimer createInnerTimer(long timeout, InnerTimerListener listener) {
		return new BasicInnerTimer(timeout, listener);
	}

}
