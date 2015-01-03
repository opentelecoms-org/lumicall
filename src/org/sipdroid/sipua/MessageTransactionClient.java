package org.sipdroid.sipua;

import org.sipdroid.sipua.ui.MessageSendingRequest;
import org.zoolu.sip.message.Message;
import org.zoolu.sip.provider.SipProvider;
import org.zoolu.sip.transaction.TransactionClient;

public class MessageTransactionClient extends TransactionClient {
	
	MessageSendingRequest msr;

	public MessageTransactionClient(SipProvider sip_provider, Message req,
			MessageAgent messageAgent, MessageSendingRequest msr) {
		super(sip_provider, req, messageAgent);
		this.msr = msr;
	}

	public MessageTransactionClient(SipProvider sip_provider, Message req,
			MessageAgent messageAgent, int i, MessageSendingRequest msr) {
		super(sip_provider, req, messageAgent, i);
		this.msr = msr;
	}
	
	MessageSendingRequest getMessageSendingRequest() {
		return msr;
	}

}
