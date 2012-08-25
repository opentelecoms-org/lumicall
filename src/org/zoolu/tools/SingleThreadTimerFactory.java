package org.zoolu.tools;

public class SingleThreadTimerFactory implements InnerTimerFactory {

	@Override
	public InnerTimer createInnerTimer(long timeout, InnerTimerListener listener) {
		return new InnerTimerST(timeout, listener);
	}

}
