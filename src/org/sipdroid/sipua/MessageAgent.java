/*
 * Copyright (C) 2005 Luca Veltri - University of Parma - Italy
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
 * Luca Veltri (luca.veltri@unipr.it)
 */

package org.sipdroid.sipua;


import org.sipdroid.sipua.ui.MessageSendingRequest;
import org.zoolu.sip.address.*;
import org.zoolu.sip.authentication.DigestAuthentication;
import org.zoolu.sip.provider.*;
import org.zoolu.sip.transaction.*;
import org.zoolu.sip.header.AuthorizationHeader;
import org.zoolu.sip.header.ProxyAuthenticateHeader;
import org.zoolu.sip.header.ProxyAuthorizationHeader;
import org.zoolu.sip.header.StatusLine;
import org.zoolu.sip.header.ViaHeader;
import org.zoolu.sip.header.WwwAuthenticateHeader;
import org.zoolu.sip.message.*;
import org.zoolu.tools.Log;
import org.zoolu.tools.LogLevel;

import java.io.*;
import java.util.logging.Logger;


/** Simple Message Agent (MA).
  * <br/>
  * It allows a user to send and receive short messages.
  */
public class MessageAgent implements SipProviderListener, TransactionClientListener
{     
   private static final int MAX_ATTEMPTS = 5;

/** Log */
   private Logger logger = Logger.getLogger(getClass().getCanonicalName());

   /** UserProfile */
   protected UserAgentProfile user_profile;

   /** SipProvider */
   protected SipProvider sip_provider;

   /** Message listener */
   protected MessageAgentListener listener;

   private int attempts;

   /** Qop for the next authentication. */
   String qop;
   
   /** Costructs a new MessageAgent. */
   public MessageAgent(SipProvider sip_provider, UserAgentProfile user_profile, MessageAgentListener listener)
   {  this.sip_provider=sip_provider;
      this.listener=listener;
      this.user_profile=user_profile;
      // init unconfigured user params
      //user_profile.setUnconfiguredAttributes(sip_provider);
   }   

   
   /** Sends a new text message. 
 * @param msr */
   public void send(String recipient, String subject, String content, MessageSendingRequest msr)
   {  send(recipient,subject,"application/text",content, msr);
   }   


   /** Sends a new message. 
 * @param msr */
   public void send(String recipient, String subject, String content_type, String content, MessageSendingRequest msr)
   {  NameAddress to_url=new NameAddress(recipient);
      NameAddress from_url=new NameAddress(user_profile.from_url);
      Message req=MessageFactory.createMessageRequest(sip_provider,to_url,from_url,subject,content_type,content);
      TransactionClient t=new MessageTransactionClient(sip_provider,req,this,msr);
      attempts = 0;  // FIXME - what if sending multiple messages?
      t.request();
   }


   /** Waits for incoming message. */
   public void receive()
   {  sip_provider.addSipProviderListener(new MethodIdentifier(SipMethods.MESSAGE),this);
   } 
   

   /** Stops receiving messages. */
   public void halt()
   {  sip_provider.removeSipProviderListener(new MethodIdentifier(SipMethods.MESSAGE));  
   } 


   // ******************* Callback functions implementation ********************

   /** When a new Message is received by the SipProvider. */
   public void onReceivedMessage(SipProvider provider, Message msg)
   {  //printLog("Message received: "+msg.getFirstLine().substring(0,msg.toString().indexOf('\r')));
	   if (msg.isRequest()) {
		   TransactionServer ts = new TransactionServer(sip_provider,msg,null);
		   int responseCode = 500;
		   String responseText = "Internal error";
		   try {
			   NameAddress sender=msg.getFromHeader().getNameAddress();
			   NameAddress recipient=msg.getToHeader().getNameAddress();
			   String subject=null;
			   if (msg.hasSubjectHeader()) subject=msg.getSubjectHeader().getSubject();
			   String content_type=null;
			   if (msg.hasContentTypeHeader()) content_type=msg.getContentTypeHeader().getContentType();
			   String content=msg.getBody();
			   if (listener!=null) {

				   listener.onMaReceivedMessage(this,sender,recipient,subject,content_type,content,msg);
			   }
			   responseCode = 200;
			   responseText = "OK";
		   } catch (Exception ex) {
			   logger.severe("Exception while handling message: " + ex.toString());
			   ex.printStackTrace();
		   }
		   ts.respondWith(MessageFactory.createResponse(msg,responseCode,responseText,null));
	   }
   }
 

   /** When the TransactionClient goes into the "Completed" state receiving a 2xx response */
   public void onTransSuccessResponse(TransactionClient tc, Message resp) 
   {  onDeliverySuccess(tc,resp.getStatusLine().getReason());
   }

   /** When the TransactionClient goes into the "Completed" state receiving a 300-699 response */
   public void onTransFailureResponse(TransactionClient tc, Message resp) {
	   StatusLine status = resp.getStatusLine();
	   int code = status.getCode();
	   if (!processAuthenticationResponse(tc, resp, code)) {
		   onDeliveryFailure(tc,resp.getStatusLine().getReason());
	   }
   }
    
   /** When the TransactionClient is (or goes) in "Proceeding" state and receives a new 1xx provisional response */
   public void onTransProvisionalResponse(TransactionClient tc, Message resp)
   {  // do nothing.
   }
      
   /** When the TransactionClient goes into the "Terminated" state, caused by transaction timeout */
   public void onTransTimeout(TransactionClient tc)
   {  onDeliveryFailure(tc,"Timeout");
   }
 
   /** When the delivery successes. */
   private void onDeliverySuccess(TransactionClient tc, String result) {  
	   logger.info("Message successfully delivered ("+result+").");
	   MessageTransactionClient mtc = (MessageTransactionClient)tc;
      Message req=tc.getRequestMessage();
      NameAddress recipient=req.getToHeader().getNameAddress();
      String subject=null;
      if (req.hasSubjectHeader()) subject=req.getSubjectHeader().getSubject();
      if (listener!=null) listener.onMaDeliverySuccess(this,recipient,subject,result);
      mtc.getMessageSendingRequest().onSuccess();
   }

   /** When the delivery fails. */
   private void onDeliveryFailure(TransactionClient tc, String result) {  
	   logger.warning("Message delivery failed ("+result+").");
	   MessageTransactionClient mtc = (MessageTransactionClient)tc;
      Message req=tc.getRequestMessage();
      NameAddress recipient=req.getToHeader().getNameAddress();
      String subject=null;
      if (req.hasSubjectHeader()) subject=req.getSubjectHeader().getSubject();
      if (listener!=null) listener.onMaDeliveryFailure(this,recipient,subject,result);
      mtc.getMessageSendingRequest().onFailure();
   }
   
   
   /**
    * The methods below were copied from RegisterAgent
    * 
    * Key differences
    * - attempts should be per-message (FIXME)
    * - TransactionClient should be per-message (FIXME)
    */
   
   
   
	private boolean generateRequestWithProxyAuthorizationheader(
			Message resp, Message req){
		if(resp.hasProxyAuthenticateHeader()
				&& resp.getProxyAuthenticateHeader().getRealmParam()
				.length() > 0){
			user_profile.realm = resp.getProxyAuthenticateHeader().getRealmParam();
			ProxyAuthenticateHeader pah = resp.getProxyAuthenticateHeader();
			String qop_options = pah.getQopOptionsParam();
			
			logger.fine("qop-options: " + qop_options);
			
			qop = (qop_options != null) ? "auth" : null;
			
			ProxyAuthorizationHeader ah = (new DigestAuthentication(
							req.getTransactionMethod(), req.getRequestLine().getAddress()
							.toString(), pah, qop, null, user_profile.username, user_profile.passwd))
					.getProxyAuthorizationHeader();
			req.setProxyAuthorizationHeader(ah);
			
			return true;
		}
		return false;
	}
	
	private boolean generateRequestWithWwwAuthorizationheader(
			Message resp, Message req){
		if(resp.hasWwwAuthenticateHeader()
				&& resp.getWwwAuthenticateHeader().getRealmParam()
				.length() > 0){		
			user_profile.realm = resp.getWwwAuthenticateHeader().getRealmParam();
			WwwAuthenticateHeader wah = resp.getWwwAuthenticateHeader();
			String qop_options = wah.getQopOptionsParam();
			
			logger.fine("qop-options: " + qop_options);
			
			qop = (qop_options != null) ? "auth" : null;
			
			AuthorizationHeader ah = (new DigestAuthentication(
							req.getTransactionMethod(), req.getRequestLine().getAddress()
							.toString(), wah, qop, null, user_profile.username, user_profile.passwd))
					.getAuthorizationHeader();
			req.setAuthorizationHeader(ah);
			return true;
		}
		return false;
	}

	private boolean handleAuthentication(int respCode, Message resp,
					     Message req) {
		
		if (user_profile.username == null || user_profile.username.length() == 0) {
			return false;
		}
		if (user_profile.passwd == null || user_profile.passwd.length() == 0) {
			return false;
		}
		
		switch (respCode) {
		case 407:
			return generateRequestWithProxyAuthorizationheader(resp, req);
		case 401:
			return generateRequestWithWwwAuthorizationheader(resp, req);
		}
		return false;
	}
		
	
	private boolean processAuthenticationResponse(TransactionClient transaction,
			Message resp, int respCode) {
		if (attempts < MAX_ATTEMPTS){
			attempts++;
			Message req = transaction.getRequestMessage();
			req.setCSeqHeader(req.getCSeqHeader().incSequenceNumber());
			ViaHeader vh=req.getViaHeader();
			String newbranch = SipProvider.pickBranch();
			vh.setBranch(newbranch);	
			req.removeViaHeader();
			req.addViaHeader(vh);

			if (handleAuthentication(respCode, resp, req)) {
				MessageTransactionClient mtc = (MessageTransactionClient)transaction;
				TransactionClient t = new MessageTransactionClient(sip_provider, req, this, 30000, mtc.getMessageSendingRequest());
				t.request();
				return true;
			}
		}
		return false;
	}

}
