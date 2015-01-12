package org.lumicall.android.sip;

import java.util.Date;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.db.UserMessage;
import org.sipdroid.sipua.ui.MessageSendingRequest;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Settings;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class NewMessage extends Activity {
	
	private static final String TAG = "NewMessage";

	static final String DEFAULT_RECIPIENT = "recipient";
	
	static final String EMPTY_RECIPIENT_VALUE = "sip:";

	private static final String DEFAULT_CONTENT_TYPE = "application/text";
	
	EditText recipient;
	EditText body;
	Button sendButton;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.sms_compose);
        setTitle(R.string.sms_compose);
        
        recipient = (EditText)findViewById(R.id.sms_recipient);
        body = (EditText)findViewById(R.id.sms_body);
        
        sendButton = (Button)findViewById(R.id.sms_send);
        sendButton.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				sendMessage();
			}
		});
        
        Intent intent = getIntent();
        String defaultRecipient = intent.getStringExtra(DEFAULT_RECIPIENT);
        if(defaultRecipient != null) {
        	recipient.setText(defaultRecipient);
        } else {
        	recipient.setText(EMPTY_RECIPIENT_VALUE);
        }
        
        if(recipient.getText().length() > 0) {
        	body.requestFocus();
        }
        
        sendButton.setEnabled(true);
   	}

	protected void sendMessage() {
		String _recipient = recipient.getText().toString();
		String _body = body.getText().toString();
		Log.i(TAG, "Message for: " + _recipient);
		Log.i(TAG, "Body: " + _body);
		
		sendButton.setEnabled(false);
		
		SIPIdentity sender = getSIPIdentity();
		
		long id = storeMessage(sender, _recipient, _body);
		MessageSendingRequest msr = new MyMessageSendingRequest(id);
		if(Receiver.engine(this).sendMessage(sender, _recipient, _body, msr)) {
			Intent intent = new Intent(this, org.lumicall.android.sip.MessageIndex.class);
			startActivity(intent);
			finish();
		} else {
			sendButton.setEnabled(true);
		}
	}
	
	protected SIPIdentity getSIPIdentity() {
		// Must use the default SIP identity for SIP-SIP calls
		SharedPreferences sipSettings = getSharedPreferences(Settings.sharedPrefsFile, Context.MODE_PRIVATE);
		long 	_sipIdentityId = Long.parseLong(sipSettings.getString(Settings.PREF_SIP, "-1"));
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();
		SIPIdentity sipIdentity = null;
		if(_sipIdentityId >= 0) {
			sipIdentity = ds.getSIPIdentity(_sipIdentityId);
		}
		if(sipIdentity == null) {
			// No default SIP identity selected in the prefs, just use the first one
			for(SIPIdentity _sipIdentity : ds.getSIPIdentities()) {
				if(_sipIdentity.isEnable()) {
					sipIdentity = _sipIdentity;
					break;
				}
			}
		}
		ds.close();
		if(!sipIdentity.isEnable())
			return null;
		return sipIdentity;
	}

	private long storeMessage(SIPIdentity sender, String _recipient,
			String _body) {
		UserMessage um = new UserMessage();
		um.setOriginLocal(true);
		um.setReceivedTimestamp(System.currentTimeMillis()/1000);
		Date messageTimestamp = new Date();
		um.setMessageTimestamp(messageTimestamp.getTime()/1000);
		//um.setSenderName( );
		um.setSenderUri(sender.getUri());
		//um.setRecipientName( );
		um.setRecipientUri(_recipient);
		//um.setSubject( );
		um.setContentType(DEFAULT_CONTENT_TYPE);
		um.setBody(_body);
		
		LumicallDataSource ds = new LumicallDataSource(Receiver.mContext);
		ds.open();
		ds.persistUserMessage(um);
		ds.close();
		
		return um.getId();
	}
	
	public class MyMessageSendingRequest implements MessageSendingRequest {
		
		long messageId;

		public MyMessageSendingRequest(long id) {
			this.messageId = id;
		}

		@Override
		public void onSuccess() {
			// FIXME - mark the message as sent in the database?
		}

		@Override
		public void onFailure() {
			// FIXME - mark the message as unsent in the database?
			NewMessage.this.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					Toast.makeText(NewMessage.this, R.string.sms_send_failure, Toast.LENGTH_LONG).show();
				}
			});
		}
		
	}

}
