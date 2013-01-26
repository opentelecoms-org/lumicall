package org.lumicall.android.reg;

import java.util.logging.Logger;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.SmsManager;


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

		switch (getResultCode()) {
		case Activity.RESULT_OK:
			logger.info("Registration SMS sent to Lumicall server, awaiting delivery report or reply");
			return;

		case SmsManager.RESULT_ERROR_GENERIC_FAILURE:
			Object _errorCode = intent.getExtras().get("errorCode");
			if(_errorCode != null)
				errorCode = String.format("Generic Failure (%s)", _errorCode.toString());
			else
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

		if (errorCode.length() == 0) {
			// Sent OK
			return;
		} else {
			logger.severe("Registration SMS sending error: " + errorCode);
			if(count > 1)
				return;
			new Thread(new RetrySender(context, dest, code, count-1)).run();
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
	      	EnrolmentService.sendValidationSMS(context, dest, code, count);
		}
		
	}


}
