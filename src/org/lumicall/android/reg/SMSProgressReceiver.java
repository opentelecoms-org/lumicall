package org.lumicall.android.reg;

import java.io.IOException;
import java.io.StringWriter;
import java.util.logging.Logger;

import org.lumicall.android.AppProperties;
import org.lumicall.android.sip.RegistrationFailedException;
import org.lumicall.android.sip.RegistrationUtil;
import org.xmlpull.v1.XmlSerializer;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.SmsManager;
import android.util.Xml;


public class SMSProgressReceiver extends BroadcastReceiver {
	
	Logger logger = Logger.getLogger(this.getClass().getCanonicalName());

	@Override
	public void onReceive(Context context, Intent intent) {
		
		if(intent.getAction().equals(EnrolmentService.ACTION_DELIVERED)) {
			if(intent.getStringExtra(EnrolmentService.SMS_REG_DEST) != null) {
            	logger.info("Registration SMS delivered to Lumicall server");
            }
			return;
		}
		
		if(!intent.getAction().equals(EnrolmentService.ACTION_SENT)) {
			logger.warning("Unexpected intent received, ignoring");
			return;
		}

		String dest = intent.getStringExtra(EnrolmentService.SMS_REG_DEST);
		String code = intent.getStringExtra(EnrolmentService.SMS_REG_CODE);
		int count = intent.getIntExtra(EnrolmentService.SMS_REG_RETRY_COUNT, 0);
		if(dest == null) {
			logger.info("Lumicall got callback about some other SMS sent");
			return;
		}
		if(code == null) {
			logger.warning("Lumicall got callback about SMS, missing validation code!");
		}
		if(count == 0) {
			logger.warning("No more retries!");
		}

		String errorCode = "";
		String errorDetail = null;

		boolean mustRetry = true;
		switch (getResultCode()) {
		case Activity.RESULT_OK:
			logger.info("Registration SMS sent to Lumicall server, awaiting delivery report or reply");
			mustRetry = false;
			break;

		case SmsManager.RESULT_ERROR_GENERIC_FAILURE:
			Object _errorCode = intent.getExtras().get("errorCode");
			if(_errorCode != null) {
				errorDetail = _errorCode.toString();
				errorCode = String.format("Generic Failure (%s)", errorDetail);
			} else
				errorCode = "Generic Failure (no reason given - check logcat)";
			break;
		case SmsManager.RESULT_ERROR_NO_SERVICE:
			errorCode = "No Service";
			break;
		case SmsManager.RESULT_ERROR_NULL_PDU:
			errorCode = "Invalid PDU";
			break;
		case SmsManager.RESULT_ERROR_RADIO_OFF:
			errorCode = "Radio Off";
			break;
		}

		if (mustRetry) {
			logger.severe("Registration SMS sending error: " + errorCode + ", retry count = " + count);
			if(count < 1) {
				final Intent _intent = new Intent(context, EnrolmentService.class);
				_intent.setAction(EnrolmentService.ACTION_SMS_FAILED);
				context.startService(_intent);
				return;
			}
			count --;
			RetrySender rs = new RetrySender(context, dest, code, count);
			new Thread(rs).start();
		}
		
		XmlSerializer serializer = Xml.newSerializer();
		StringWriter writer = new StringWriter();
		try {
			serializer.setOutput(writer);
			serializer.startDocument("UTF-8", true);
			String ns = RegistrationUtil.NS;
			serializer.startTag(ns, "smsSendingReport");
			RegistrationUtil.serializeProperty(serializer, ns, "timestamp", Long.toString(System.currentTimeMillis()/1000));
			RegistrationUtil.serializeProperty(serializer, ns, "dest", dest);
			RegistrationUtil.serializeProperty(serializer, ns, "code", code);
			RegistrationUtil.serializeProperty(serializer, ns, "resultCode", getResultCode());
			if (errorDetail != null) {
				RegistrationUtil.serializeProperty(serializer, ns, "errorCode", errorDetail);
			}
			serializer.endTag(ns, "smsSendingReport");
			serializer.endDocument();
			String reportBody = writer.toString();
			
			AppProperties props = new AppProperties(context);
			RegistrationUtil.submitMessage(props, "smsSent", reportBody, null);
		} catch (Exception e) {
			logger.severe("error constructing SMS report");
		}
	}

	class RetrySender implements Runnable {
		
		private String dest;
		private String code;
		private int count;
		private Context context;

		public RetrySender(Context context, String dest, String code, int count) {
			this.context = context;
			this.dest = dest;
			this.code = code;
			this.count = count;
		}

		@Override
		public void run() {
			try {
				Thread.sleep(EnrolmentService.SMS_INTERVAL * 1000);
			} catch (InterruptedException e) {
				logger.info("Interrupted while sleeping for retry");
			}
			logger.info("Initiating retry, count = " + count);
	      	EnrolmentService.sendValidationSMS(context, dest, code, count);
		}
		
	}


}
