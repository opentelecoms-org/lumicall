package org.lumicall.android.sip;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.UserMessage;
import org.sipdroid.sipua.ui.Receiver;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

public class MessageIndex extends Activity {
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.sms_index);
        setTitle(R.string.sms_index);
        
        loadMessages();
	}

	private void loadMessages() {
		ListView smsList = (ListView)findViewById(R.id.sms_list);
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();
		List<UserMessage> messages = ds.getUserMessages();
		ds.close();
		
		ArrayAdapter<UserMessage> adapter = new MessageArrayAdapter(this);
		for(UserMessage um : messages) {
			adapter.add(um);
		}
		
		smsList.setAdapter(adapter);
	}
	
	public class MessageArrayAdapter extends ArrayAdapter<UserMessage> {
		
		public MessageArrayAdapter(Context context) {
			super(context, android.R.layout.simple_list_item_1);
			
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			View view = MessageIndex.this.getLayoutInflater().inflate(R.layout.sms_index_item, parent, false);
			UserMessage um = getItem(position);
			String peerUri = null;
			String peerName = null;
			if(um.isOriginLocal()) {
				peerUri = um.getRecipientUri();
				peerName = um.getRecipientName();
			} else {
				peerUri = um.getSenderUri();
				peerName = um.getSenderName();
			}
			StringBuilder peer = new StringBuilder("");
			if(peerName != null) {
				peer.append(peerName).append(" ");
			}
			peer.append("<").append(peerUri).append(">");
			
			DateFormat df = null;
			long ts = um.getReceivedTimestamp();
			long now = System.currentTimeMillis()/1000;
			long m1y = now - 365*24*60*60;
			if(ts < m1y) {
				df = new SimpleDateFormat("yyyy-MM-dd");
			} else {
				long m1d = now - 24*60*60;
				if(ts < m1d) {
					df = new SimpleDateFormat("MMM dd");
				} else {
					df = new SimpleDateFormat("HH:mm");
				}
			}
			String fDate = df.format(ts*1000);
			
			TextView peerView = (TextView)view.findViewById(R.id.sms_index_item_peer);
			TextView subjectView = (TextView)view.findViewById(R.id.sms_index_item_subject);
			TextView bodyView = (TextView)view.findViewById(R.id.sms_index_item_body);
			TextView dateView = (TextView)view.findViewById(R.id.sms_index_item_date);
			
			peerView.setText(peer);
			String subject = um.getSubject();
			if(subject != null && subject.length() > 0) {
				subjectView.setText(subject);
			} else {
				subjectView.setVisibility(View.INVISIBLE);
				subjectView.setHeight(0);
			}
			bodyView.setText(um.getBody());
			dateView.setText(fDate);
			
			return view;
		}
	}

}
