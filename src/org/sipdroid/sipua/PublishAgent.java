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


import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

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
import org.zoolu.tools.LogLevel;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
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
	 * User realm.
	 */
	String realm;
	/**
	 * Nonce for the next authentication.
	 */
	String next_nonce;
	/**
	 * User's passwd.
	 */
	String passwd;
	/**
	 * Expiration time.
	 */
	int expire_time;

	private Logger logger = Logger.getLogger(getClass().getCanonicalName());
	/**
	 * SipProvider
	 */
	protected SipProvider sip_provider;
	Context context;

	public PublishAgent(SipProvider sip_provider, UserAgentProfile user_profile, String username, String realm, String passwd, Context context) {
		this.sip_provider = sip_provider;
		this.user_profile = user_profile;
		this.username = username;
		this.realm = realm;
		this.passwd = passwd;
		this.next_nonce = null;
		this.context = context;

	}

	public void publish() {
		this.expire_time = SipStack.default_expires;
		this.publish("open", expire_time, "");
	}

	public void publish(String status, int expireTime, String note) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		Boolean publish_enable_status = prefs.getBoolean("publish_enable", true);
		if (publish_enable_status == true && (status=="open" || status=="close")) {
			MessageDigest md = null;
			String tupleId;
			try {
				md = MessageDigest.getInstance("MD5");
				tupleId =  md.digest(user_profile.username.getBytes()).toString();
			} catch (NoSuchAlgorithmException e) {
				tupleId = user_profile.username;
				e.printStackTrace();
			}
			String from = user_profile.username;
			String entity = "sip:" + user_profile.username;
			String xml =
					"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
							"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
							" entity=\"" + entity + "\">" +
							"<tuple id=\"" + tupleId + "\">" +
							"<status>" +
							"<basic>" + status + "</basic>" +
							"<note>" + note + "</note>" +
							"</status>" +
							"</tuple>" +
							"</presence>";
			MessageFactory msgf = new MessageFactory();
			Message req = msgf.createPublishRequest(sip_provider, new NameAddress(from), "presence", expireTime, "application/pidf+xml", xml);
			TransactionClient t = new TransactionClient(sip_provider, req, this);
			t.request();
		}
	}

	public void unPublish() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		String from = user_profile.username;
		Boolean publish_enable_status = prefs.getBoolean("publish_enable", true);
		if (publish_enable_status == true) {
			MessageFactory msgf = new MessageFactory();
			Message req = msgf.createPublishRequest(sip_provider, new NameAddress(from), "presence", 0, null, null);
			TransactionClient t = new TransactionClient(sip_provider, req, this);
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
		int code = status.getCode();
		logger.fine(String.valueOf(status));
		logger.fine(String.valueOf(code));
	}

	public void onTransFailureResponse(TransactionClient tc, Message resp) {
		StatusLine status = resp.getStatusLine();
		int code = status.getCode();
		processAuthenticationResponse(tc, resp, code);
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
	}

	public void onTransProvisionalResponse(TransactionClient tc, Message resp) {
		// FIXME
		StatusLine status = resp.getStatusLine();
		int code = status.getCode();
		logger.fine(String.valueOf(status));
		logger.fine(String.valueOf(code));
	}

}
