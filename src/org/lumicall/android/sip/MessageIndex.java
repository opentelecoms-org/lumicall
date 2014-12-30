package org.lumicall.android.sip;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.UserMessage;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.RegisterService;
import org.sipdroid.sipua.ui.SIPUri;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

public class MessageIndex extends Activity {
	
	public static final int FIRST_MENU_ID = Menu.FIRST;
	public static final int NEW_MESSAGE = FIRST_MENU_ID + 1;
	public static final String MESSAGE_LIST_CHANGE = "org.lumicall.android.sip.MESSAGE_LIST_CHANGE";
	
	ListView smsList;
	ArrayAdapter<UserMessage> adapter;
	
	private MyReceiver mReceiver;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.sms_index);
        setTitle(R.string.sms_index);
        
        smsList = (ListView)findViewById(R.id.sms_list);
		
        adapter = null;
        loadMessages();
        
		smsList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position,
					long id) {
				replyTo(position);
			}
		});
		smsList.smoothScrollToPosition(adapter.getCount());
		smsList.setTranscriptMode(ListView.TRANSCRIPT_MODE_NORMAL);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		boolean result = super.onCreateOptionsMenu(menu);

		MenuItem m = menu.add(0, NEW_MESSAGE, 0, R.string.menu_new_sms);
						
		return result;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		boolean result = super.onOptionsItemSelected(item);
		Intent intent = null;

		switch (item.getItemId()) {
		case NEW_MESSAGE: {
			intent = new Intent(this, org.lumicall.android.sip.NewMessage.class);
			startActivity(intent);
		}
		break;
		}

		return result;
	}
	
	private void loadMessages() {
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();
		List<UserMessage> messages = ds.getUserMessages();
		ds.close();
		
		if(adapter == null) {
			adapter = new MessageArrayAdapter(this);
			smsList.setAdapter(adapter);
		} else {
			adapter.clear();
		}
		for(UserMessage um : messages) {
			adapter.add(um);
		}
	}
	
	private void replyTo(int position) {
		UserMessage um = adapter.getItem(position);
		String recipient = null;
		if(um.isOriginLocal()) {
			// If it is a message we sent, we don't reply to ourselves,
			// we send another message to the same recipient
			recipient = um.getRecipientUri();
		} else {
			// If it is a message we received, reply to the sender
			recipient = um.getSenderUri();
		}
		Intent intent = new Intent(this, org.lumicall.android.sip.NewMessage.class);
		intent.putExtra(NewMessage.DEFAULT_RECIPIENT, recipient);
		startActivity(intent);
		finish();
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
			boolean outgoing = um.isOriginLocal();
			if(outgoing) {
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
			
			// Right-align the outgoing messages
			if(outgoing) {
				LayoutParams lp = new LinearLayout.LayoutParams(
		                LinearLayout.LayoutParams.FILL_PARENT,
		                LinearLayout.LayoutParams.WRAP_CONTENT);
				for(TextView v : new TextView[] { peerView, subjectView, bodyView, dateView }) {
					v.setLayoutParams(lp);
					v.setGravity(Gravity.RIGHT);
				}
			}
			
			return view;
		}
	}
	
	@Override
    protected void onResume() {
        super.onResume();
        IntentFilter mFilter = new IntentFilter();
        // add action to this filters
        mFilter.addAction(MESSAGE_LIST_CHANGE);

        // Initialize receiver
        mReceiver = new MyReceiver();
        // register when activity is resumed
        registerReceiver(mReceiver, mFilter);
    }

    @Override
    protected void onPause() {
        super.onPause();
        // unregister when activity is paused
        if(mReceiver != null){
            unregisterReceiver(mReceiver);
        }
    }

    class MyReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            loadMessages();
        }

    }

}
