/*
 * Copyright (C) 2016 Pranav Jain
 *
 * This source code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this source code; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author(s):
 * Pranav Jain (contact@pranavjain.me)
 */

package org.sipdroid.sipua;


import org.lumicall.android.Util;
import org.zoolu.sip.address.NameAddress;
import org.zoolu.sip.authentication.DigestAuthentication;
import org.zoolu.sip.header.AuthorizationHeader;
import org.zoolu.sip.header.ProxyAuthenticateHeader;
import org.zoolu.sip.header.ProxyAuthorizationHeader;
import org.zoolu.sip.header.StatusLine;
import org.zoolu.sip.header.ViaHeader;
import org.zoolu.sip.header.WwwAuthenticateHeader;
import org.zoolu.sip.message.Message;
import org.zoolu.sip.message.MessageFactory;
import org.zoolu.sip.provider.SipProvider;
import org.zoolu.sip.provider.SipStack;
import org.zoolu.sip.transaction.TransactionClient;
import org.zoolu.sip.transaction.TransactionClientListener;
import org.zoolu.tools.BasicTimerFactory;
import org.zoolu.tools.LogLevel;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.MessageFormat;
import java.util.logging.Logger;


public class PublishAgent implements TransactionClientListener {

	/**
	 * UserProfile
	 */
	protected UserAgentProfile user_profile;
	/**
	 * User name.
	 */
	String username;
	/**
	 * User's passwd.
	 */
	String passwd;
	/**
	 * Publish Setting.
	 */
	boolean enablePublish;
	private Logger logger = Logger.getLogger(getClass().getCanonicalName());
	/**
	 * SipProvider
	 */
	protected SipProvider sip_provider;

	public PublishAgent(SipProvider sip_provider, UserAgentProfile user_profile, boolean enablePublish) {
		this.sip_provider = sip_provider;
		this.user_profile = user_profile;
		this.username = user_profile.username;
		this.passwd = user_profile.passwd;
		this.enablePublish = enablePublish;
	}

	public void publish() {
		this.publish(BasicStatus.OPEN, SipStack.default_expires, "Open");
	}

	public void publish(BasicStatus status, int expireTime, String note) {
		if (enablePublish == true) {
			String tupleId;
			try {
				MessageDigest md = MessageDigest.getInstance("MD5");
				byte[] _digest = md.digest(user_profile.username.getBytes());
				tupleId = Util.byteToHexString(_digest, 0, _digest.length);
			} catch (NoSuchAlgorithmException e) {
				tupleId = user_profile.username;
				e.printStackTrace();
			}
			String entity = "sip:" + user_profile.from_url;
			String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" +
					"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" " +
					"xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" " +
					"xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\"" +
					" entity=\"" + entity + "\">" +
					" <dm:person id=\"" + "PID-" + tupleId + "\">" +
					"<rpid:activities/>" +
					"</dm:person>" +
					"<tuple id=\"" + "TID-" + tupleId + "\">" +
					"<status>" +
					"<basic>" + status.pidf() + "</basic>" +
					"</status>" +
					"<contact>" + entity + "</contact>" +
					"<note>" + note + "</note>" +
					"</tuple>" +
					"</presence>";
			MessageFactory msgf = new MessageFactory();
			Message req = msgf.createPublishRequest(sip_provider, new NameAddress(user_profile.from_url), "presence", expireTime, "application/pidf+xml", xml);
			TransactionClient t = new TransactionClient(sip_provider, req, this, 30000);
			t.request();
		}
	}

	public void unPublish() {
		if (enablePublish == true) {
			MessageFactory msgf = new MessageFactory();
			Message req = msgf.createPublishRequest(sip_provider, new NameAddress(user_profile.from_url), "presence", 0, null, null);
			TransactionClient t = new TransactionClient(sip_provider, req, this, 30000);
			t.request();
		}
	}

	private boolean generateRequestWithProxyAuthorizationheader(Message resp, Message req) {
		ProxyAuthenticateHeader pah = resp.getProxyAuthenticateHeader();
		ProxyAuthorizationHeader ah1 = (new DigestAuthentication(
				req.getTransactionMethod(), req.getRequestLine().getAddress()
				.toString(), pah, null, null, username, passwd))
				.getProxyAuthorizationHeader();
		req.setProxyAuthorizationHeader(ah1);
		return true;
	}

	public void onTransSuccessResponse(TransactionClient tc, Message resp) {
		StatusLine status = resp.getStatusLine();
		String log = MessageFormat.format("onTransSuccessResponse: status {0}: {1}", status.getCode(), status);
		logger.fine(log);
	}

	public void onTransFailureResponse(TransactionClient tc, Message resp) {
		StatusLine status = resp.getStatusLine();
		int code = status.getCode();
		String log = MessageFormat.format("onTransFailureResponse: status {0}: {1}", code, status);
		logger.fine(log);
		if (code == 401 || code == 407) {
			processAuthenticationResponse(tc, resp, code);
		}
	}

	private boolean processAuthenticationResponse(TransactionClient transaction, Message resp, int respCode) {
		Message req = transaction.getRequestMessage();
		req.setCSeqHeader(req.getCSeqHeader().incSequenceNumber());
		ViaHeader vh = req.getViaHeader();
		String newbranch = SipProvider.pickBranch();
		vh.setBranch(newbranch);
		req.removeViaHeader();
		req.addViaHeader(vh);
		handleAuthentication(respCode, resp, req);
		TransactionClient t = new TransactionClient(sip_provider, req, this, 30000);
		t.request();
		return true;
	}

	private boolean generateRequestWithWwwAuthorizationheader(Message resp, Message req) {
		WwwAuthenticateHeader wah = resp.getWwwAuthenticateHeader();
		AuthorizationHeader ah = (new DigestAuthentication(
				req.getTransactionMethod(), req.getRequestLine().getAddress()
				.toString(), wah, null, null, user_profile.username, user_profile.passwd))
				.getAuthorizationHeader();
		req.setAuthorizationHeader(ah);
		return true;
	}

	private boolean handleAuthentication(int respCode, Message resp, Message req) {
		switch (respCode) {
			case 407:
				return generateRequestWithProxyAuthorizationheader(resp, req);
			case 401:
				return generateRequestWithWwwAuthorizationheader(resp, req);
		}
		return false;
	}

	@Override
	public void onTransTimeout(TransactionClient tc) {
		//FIXME
		logger.warning("onTransTimeout : Timeout!");
	}

	public void onTransProvisionalResponse(TransactionClient tc, Message resp) {
		//FIXME
		StatusLine status = resp.getStatusLine();
		String log = MessageFormat.format("onTransProvisionalResponse: status {0}: {1}", status.getCode(), status);
		logger.fine(log);
	}
}
