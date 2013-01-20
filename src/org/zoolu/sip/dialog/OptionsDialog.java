/*
 * Copyright (C) 2013 Daniel Pocock <daniel@pocock.com.au>
 * Copyright (C) 2005 Luca Veltri - University of Parma - Italy
 * 
 * This file is part of MjSip (http://www.mjsip.org)
 * 
 * MjSip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * MjSip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MjSip; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author(s):
 * Luca Veltri (luca.veltri@unipr.it)
 * Nitin Khanna, Hughes Systique Corp. (Reason: Android specific change, optmization, bug fix) 
 */

/* Modified by:
 * Daina Interrante (daina.interrante@studenti.unipr.it)
 */

package org.zoolu.sip.dialog;

import org.zoolu.sip.address.*;
import org.zoolu.sip.transaction.*;
import org.zoolu.sip.message.*;
import org.zoolu.sip.header.*;
import org.zoolu.sip.provider.*;
import org.zoolu.tools.LogLevel;

/**
 * OptionsDialog.
 * 
 * This is adapted from SubscriberDialog
 * 
 * It's doesn't handle 401 or 407 authentication.  The code
 * from ExtendedInviteDialog could be adapted for that purpose.
 * 
 */
public class OptionsDialog extends Dialog implements
		TransactionClientListener {
	/** String "active" */
	protected static final String ACTIVE = "active";
	/** String "pending" */
	protected static final String PENDING = "pending";
	/** String "terminated" */
	protected static final String TERMINATED = "terminated";

	/** The current subscribe method */
	// Message subscribe=null;
	/** The subscribe transaction */
	TransactionClient options_transaction;

	/** The notify transaction */
	// TransactionServer notify_transaction=null;
	/** The SubscriberDialog listener */
	OptionsDialogListener listener;

	/** Internal state D_INIT */
	protected static final int D_INIT = 0;
	/** Internal state D_SENT */
	protected static final int D_SENT = 1;
	/** Internal state D_OPTIONSD */
	protected static final int D_ACCEPTED = 2;
	/** Internal state D_PENDING */
	protected static final int D_PENDING = 3;
	/** Internal state D_ACTIVE */
	protected static final int D_ACTIVE = 4;
	/** Internal state D_TERMINATED */
	protected static final int D_TERMINATED = 9;

	/** Gets the dialog state */
	protected String getStatusDescription() {
		switch (status) {
		case D_INIT:
			return "D_INIT";
		case D_SENT:
			return "D_SENT";
		case D_ACCEPTED:
			return "D_ACCEPTED";
		case D_PENDING:
			return "D_PENDING";
		case D_ACTIVE:
			return "D_ACTIVE";
		case D_TERMINATED:
			return "D_TERMINATED";
		default:
			return null;
		}
	}
	
	protected int getStatus()
	{
		return status;
	}

	// *************************** Public methods **************************

	/** Whether the dialog is in "early" state. */
	public boolean isEarly() {
		return (status < D_ACCEPTED);
	}

	/** Whether the dialog is in "confirmed" state. */
	public boolean isConfirmed() {
		return (status >= D_ACCEPTED && status < D_TERMINATED);
	}

	/** Whether the dialog is in "active" state. */
	public boolean isTerminated() {
		return (status == D_TERMINATED);
	}

	/** Whether the subscription is "pending". */
	public boolean isSubscriptionPending() {
		return (status >= D_ACCEPTED && status < D_ACTIVE);
	}

	/** Whether the subscription is "active". */
	public boolean isSubscriptionActive() {
		return (status == D_ACTIVE);
	}

	/** Whether the subscription is "terminated". */
	public boolean isSubscriptionTerminated() {
		return (status == D_TERMINATED);
	}

	// **************************** Costructors ****************************

	/** Creates a new OptionsDialog. */
	public OptionsDialog(SipProvider sip_provider, /*
														 * String subscriber,
														 * String contact,
														 */
	 OptionsDialogListener listener) {
		super(sip_provider);
		this.listener = listener;
		this.options_transaction = null;
		// this.from_url=new NameAddress(subscriber);
		// if (contact!=null) this.contact_url=new NameAddress(contact);
		// else this.contact_url=from_url;
		
		changeStatus(D_INIT);
	}

	// *************************** Public methods **************************

	/**
	 * Sends a new OPTIONS request (starts a new subscription). It also
	 * initializes the dialog state information.
	 * 
	 * @param target
	 *            the target url (and display name)
	 * @param caller
	 *            the `caller' url (and display name)
	 * @param contact
	 *            the contact url OR the contact user-name
	 *            
	 * Each URL must include the URI prefix, e.g. sip:
	 */
	public void options(String target, String caller, String contact) {
		printLog("inside options(target=" + target + ",caller="
				+ caller + ",contact=" + contact 
				+ ")", LogLevel.MEDIUM);
		SipURL request_uri = new SipURL(target);
		NameAddress to_url = new NameAddress(target);
		NameAddress from_url = new NameAddress(caller);
		NameAddress contact_url;
		if (contact != null)
			contact_url = new NameAddress(contact);
		else
			contact_url = from_url;
		String content_type = null;
		String body = null;
		Message req = MessageFactory.createRequest(sip_provider,
				"OPTIONS",
				request_uri, to_url, from_url, contact_url, body);
		req.setHeader(new AcceptHeader("application/sdp"));
		//req.setExpiresHeader(new ExpiresHeader(expires));
		options(req);
	}

	/**
	 * Sends a new OPTIONS request (starts a new subscription). It also
	 * initializes the dialog state information.
	 * 
	 * @param req
	 *            the OPTIONS message
	 */
	public void options(Message req) {
		printLog("inside options(req)", LogLevel.MEDIUM);
		if (statusIs(D_TERMINATED)) {
			printLog("OPTIONS req already terminated: request aborted",
					LogLevel.MEDIUM);
			return;
		}
		// else
		if (statusIs(D_INIT)) {
			changeStatus(D_SENT);
		}
		update(UAC, req);
		// start client transaction
		options_transaction = new TransactionClient(sip_provider, req, this);
		options_transaction.request();
	}


	// ************** Inherited from TransactionClientListener **************

	/**
	 * When the TransactionClient is (or goes) in "Proceeding" state and
	 * receives a new 1xx provisional response
	 */
	public void onTransProvisionalResponse(TransactionClient tc, Message resp) {
		printLog("onTransProvisionalResponse()", LogLevel.MEDIUM);
		// do nothing.
	}

	/**
	 * When the TransactionClient goes into the "Completed" state receiving a
	 * 2xx response
	 */
	public void onTransSuccessResponse(TransactionClient tc, Message resp) {
		printLog("onTransSuccessResponse()", LogLevel.MEDIUM);
		if (!statusIs(D_ACTIVE)) {
			changeStatus(D_ACCEPTED);
			update(UAC, resp);
			StatusLine status_line = resp.getStatusLine();
			if (listener != null)
				listener.onDlgOptionsSuccess(this, status_line.getCode(),
						status_line.getReason(), resp);
		} else if (statusIs(D_ACTIVE)) {
			StatusLine status_line = resp.getStatusLine();
			if (listener != null)
				listener.onDlgOptionsSuccess(this, status_line.getCode(),
						status_line.getReason(), resp);
		}
	}

	/**
	 * When the TransactionClient goes into the "Completed" state receiving a
	 * 300-699 response
	 */
	public void onTransFailureResponse(TransactionClient tc, Message resp) {
		printLog("onTransFailureResponse()", LogLevel.MEDIUM);
		changeStatus(D_TERMINATED);
		StatusLine status_line = resp.getStatusLine();
		if (listener != null)
			listener.onDlgOptionsFailure(this, status_line.getCode(),
					status_line.getReason(), resp);
	}

	/**
	 * When the TransactionClient goes into the "Terminated" state, caused by
	 * transaction timeout
	 */
	public void onTransTimeout(TransactionClient tc) {
		printLog("onTransTimeout()", LogLevel.MEDIUM);
		changeStatus(D_TERMINATED);
		if (listener != null)
			listener.onDlgOptionsTimeout(this);
	}

	// ***************** Inherited from SipProviderListener *****************

	/** When a new Message is received by the SipProvider. */
	public void onReceivedMessage(SipProvider sip_provider, Message msg) {
		printLog("onReceivedMessage()", LogLevel.MEDIUM);
		if (statusIs(D_TERMINATED)) {
			printLog("options already terminated: message discarded",
					LogLevel.MEDIUM);
			return;
		}
			printLog("message unexpected: message discarded",
					LogLevel.HIGH);

	}

	// **************************** Logs ****************************/

	/** Adds a new string to the default Log */
	protected void printLog(String str, int level) {
		super.printLog("OptionsDialog#" + dialog_sqn + ": " + str, level
					+ SipStack.LOG_LEVEL_DIALOG);
	}

}
