/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
 * Copyright (C) 2008 Hughes Systique Corporation, USA (http://www.hsc.com)
 * 
 * This file is part of Sipdroid (http://www.sipdroid.org)
 * 
 * Sipdroid is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
 */

package org.sipdroid.sipua.ui;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.SocketAddress;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Logger;

import org.lumicall.android.R;
import org.lumicall.android.db.DBObject;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.PTTChannel;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.sip.DialCandidateHelper;
import org.lumicall.android.sip.MessageIndex;
import org.lumicall.android.sip.RegisterAccount;
import zorg.SRTP;
import org.sipdroid.codecs.Codecs;
import org.sipdroid.media.RtpStreamReceiver;
import org.sipdroid.media.RtpStreamSender;
import org.sipdroid.sipua.SipdroidEngine;
import org.sipdroid.sipua.UserAgent;
import org.zoolu.net.TcpSocket;
import org.zoolu.sdp.SRTPKeySpec;
import org.zoolu.sdp.SessionDescriptor;
import org.zoolu.tools.Random;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.DialogInterface.OnDismissListener;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.Cursor;
import android.database.CursorWrapper;
import android.graphics.Color;
import android.media.AudioManager;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Looper;
import android.preference.PreferenceManager;
import android.provider.CallLog.Calls;
import android.provider.Contacts.People;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnKeyListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.CursorAdapter;
import android.widget.Filterable;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

import org.omnidial.harvest.DialCandidate;

/////////////////////////////////////////////////////////////////////
// this the main activity of Sipdroid
// for modifying it additional terms according to section 7, GPL apply
// see ADDITIONAL_TERMS.txt
/////////////////////////////////////////////////////////////////////
public class Sipdroid extends ActionBarActivity implements OnDismissListener {
	
	Logger logger = Logger.getLogger(getClass().getName());

	public static final boolean release = false;
	public static final boolean market = false;

	/* Following the menu item constants which will be used for menu creation */
	public static final int FIRST_MENU_ID = Menu.FIRST;
	public static final int CONFIGURE_MENU_ITEM = FIRST_MENU_ID + 1;
	public static final int ABOUT_MENU_ITEM = FIRST_MENU_ID + 2;
	public static final int EXIT_MENU_ITEM = FIRST_MENU_ID + 3;
	public static final int ADD_ACCOUNT_MENU_ITEM = FIRST_MENU_ID + 4;
	
	private static final int DLG_IDENTITY_ID = 1;
	private static final int DLG_PTT_CHANNEL_ID = 2;

	private static AlertDialog m_AlertDlg;
	AutoCompleteTextView sip_uri_box;
	// AutoCompleteTextView sip_uri_box2;
	Button buttonSend = null;
	
	Button buttonIdentity;
	SIPIdentity chosenIdentity = null;
	
	Button buttonPTTChannel;
	PTTChannel chosenPTTChannel = null;
	
	Button buttonPTTSend;
	
	LinearLayout pttControls;
	
	Button smsButton;
	
	@Override
	public void onStart() {
		super.onStart();
		TcpSocket.setUseRecentTLSVersions(android.os.Build.VERSION.SDK_INT >= 16);
		Receiver.engine(this).registerMore();
	    ContentResolver content = getContentResolver();
	    Cursor cursor = content.query(Calls.CONTENT_URI,
	            PROJECTION, Calls.NUMBER+" like ?", new String[] { "%@%" }, Calls.DEFAULT_SORT_ORDER);
	    CallsAdapter adapter = new CallsAdapter(this, cursor);
	    sip_uri_box.setAdapter(adapter);
	    //sip_uri_box2.setAdapter(adapter);
	}
	
	public static class CallsCursor extends CursorWrapper {
		List<String> list;
		
		public int getCount() {
			return list.size();
		}
		
		public String getString(int i) {
			return list.get(getPosition());
		}
		
		public CallsCursor(Cursor cursor) {
			super(cursor);
			list = new ArrayList<String>();
			for (int i = 0; i < cursor.getCount(); i++) {
				moveToPosition(i);
 		        String phoneNumber = super.getString(1);
		        String cachedName = super.getString(2);
		        if (cachedName != null && cachedName.trim().length() > 0)
		        	phoneNumber += " <" + cachedName + ">";
		        if (list.contains(phoneNumber)) continue;
				list.add(phoneNumber);
			}
			moveToFirst();
		}
		
	}
	
	public static class CallsAdapter extends CursorAdapter implements Filterable {
	    public CallsAdapter(Context context, Cursor c) {
	        super(context, c);
	        mContent = context.getContentResolver();
	    }
	
	    public View newView(Context context, Cursor cursor, ViewGroup parent) {
	        final LayoutInflater inflater = LayoutInflater.from(context);
	        final TextView view = (TextView) inflater.inflate(
	                android.R.layout.simple_dropdown_item_1line, parent, false);
	    	String phoneNumber = cursor.getString(1); 
	        view.setText(phoneNumber);
	        return view;
	    }
	
	    @Override
	    public void bindView(View view, Context context, Cursor cursor) {
	    	String phoneNumber = cursor.getString(1);
	        ((TextView) view).setText(phoneNumber);
	    }
	
	    @Override
	    public String convertToString(Cursor cursor) {
	    	String phoneNumber = cursor.getString(1);
	    	if (phoneNumber.contains(" <"))
	    		phoneNumber = phoneNumber.substring(0,phoneNumber.indexOf(" <"));
	        return phoneNumber;
	    }
	
	    @Override
	    public Cursor runQueryOnBackgroundThread(CharSequence constraint) {
	        if (getFilterQueryProvider() != null) {
	            return new CallsCursor(getFilterQueryProvider().runQuery(constraint));
	        }
	
	        StringBuilder buffer;
	        String[] args;
	        buffer = new StringBuilder();
	        buffer.append(Calls.NUMBER);
	        buffer.append(" LIKE ? OR ");
	        buffer.append(Calls.CACHED_NAME);
	        buffer.append(" LIKE ?");
	        String arg = "%" + (constraint != null && constraint.length() > 0?
       				constraint.toString() : "@") + "%";
	        args = new String[] { arg, arg};
	
	        return new CallsCursor(mContent.query(Calls.CONTENT_URI, PROJECTION,
	                buffer.toString(), args,
	                Calls.NUMBER + " asc"));
	    }
	
	    private ContentResolver mContent;        
	}
	
	private static final String[] PROJECTION = new String[] {
        Calls._ID,
        Calls.NUMBER,
        Calls.CACHED_NAME
	};

	@Override
	public void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		
		//requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.sipdroid);
		sip_uri_box = (AutoCompleteTextView) findViewById(R.id.txt_callee);
		//sip_uri_box2 = (AutoCompleteTextView) findViewById(R.id.txt_callee2);
		sip_uri_box.setOnKeyListener(new OnKeyListener() {
		    public boolean onKey(View v, int keyCode, KeyEvent event) {
		        if (event.getAction() == KeyEvent.ACTION_DOWN &&
		        		keyCode == KeyEvent.KEYCODE_ENTER) {
		          call_menu(sip_uri_box);
		          return true;
		        }
		        return false;
		    }
		});
		sip_uri_box.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				call_menu(sip_uri_box);
			}
		});
		/* sip_uri_box2.setOnKeyListener(new OnKeyListener() {
		    public boolean onKey(View v, int keyCode, KeyEvent event) {
		        if (event.getAction() == KeyEvent.ACTION_DOWN &&
		        		keyCode == KeyEvent.KEYCODE_ENTER) {
		          call_menu(sip_uri_box2);
		          return true;
		        }
		        return false;
		    }
		});
		sip_uri_box2.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				call_menu(sip_uri_box2);
			}
		}); */
		on(this,true);

		Button contactsButton = (Button) findViewById(R.id.contacts_button);
		contactsButton.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				Intent myIntent = new Intent(Intent.ACTION_DIAL);
				startActivity(myIntent);
			}
		});
		
		buttonSend = (Button) findViewById(R.id.ButtonCallSend);
		buttonSend.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				call_menu(sip_uri_box);
			}
		});
		
		buttonIdentity = (Button) findViewById(R.id.ButtonIdentityMenu);
		buttonIdentity.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				showDialog(DLG_IDENTITY_ID);
			}
		});

		setDefaultIdentity();
		
		buttonPTTChannel = (Button) findViewById(R.id.ButtonPTTChannelMenu);
		buttonPTTChannel.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				showDialog(DLG_PTT_CHANNEL_ID);
			}
		});
		
		buttonPTTSend = (Button) findViewById(R.id.ButtonPTTSend);
		buttonPTTSend.setOnTouchListener(new Button.OnTouchListener() {
			@Override
			public boolean onTouch(View arg0, MotionEvent arg1) {
				logger.info("Got motion event: " + arg1);
				if((arg1.getActionMasked() == MotionEvent.ACTION_DOWN ||
						arg1.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN)) {
					logger.info("Got motion event for TX: " + arg1);
					if(chosenPTTChannel != null)
						startTX();
				} else if((arg1.getActionMasked() == MotionEvent.ACTION_UP) ||
						(arg1.getActionMasked() == MotionEvent.ACTION_POINTER_UP) ||
						(arg1.getActionMasked() == MotionEvent.ACTION_CANCEL)) {
					logger.info("Got motion event for RX: " + arg1);
					if(chosenPTTChannel != null)
						startRX();
				}
				return false;
			}
		});
		
		pttControls = (LinearLayout) findViewById(R.id.PTTControls);
		boolean enablePTT = PreferenceManager.getDefaultSharedPreferences(this).getBoolean(Settings.PREF_PTT_ENABLE, Settings.DEFAULT_PTT_ENABLE);
		if(enablePTT) {
			pttControls.setVisibility(View.VISIBLE);
		} else {
			pttControls.setVisibility(View.INVISIBLE);
		}

		final Context mContext = this;
		final OnDismissListener listener = this;
		
		updatePTTChannel();
		
		smsButton = (Button) findViewById(R.id.sms_button);
		smsButton.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				final Intent intent = new Intent(Sipdroid.this,
						MessageIndex.class);
				startActivity(intent);
				finish();
			}
		});
		
	}
	
	RtpStreamSender sender = null;
	Codecs.Map cmap = null;
	SRTP srtp = null;
	
    final static String eol = System.getProperty("line.separator"); 
	final static String offers = "v=0" + eol +
			"o=root 2019150241 2019150241 IN IP4 127.0.0.1" + eol +
			"s=Lumicall" + eol +
			"c=IN IP4 127.0.0.1" + eol +
			"t=0 0" + eol +
			"m=audio 5004 RTP/AVP 8" + eol +
			"a=rtpmap:8 PCMA/8000" + eol +
			"a=ptime:20" + eol +
			"a=sendrecv";
	WifiManager.MulticastLock multicastLock = null;
	
	private void updatePTTChannel() {
		if(receiver != null) {
			receiver.halt();
			receiver = null;
		}
		if(sender != null) {
			sender.halt();
			sender = null;
		}
		if(srtp != null) {
			srtp.endSession();
			srtp = null;
		}
		if(socket != null) {
			socket.close();
			socket = null;
		}
		if(multicastLock != null) {
			if(multicastLock.isHeld())
				multicastLock.release();
			multicastLock = null;
		}
		
		
		startRX();
	}
	
	protected void prepareMulticastSocket() throws IOException {
		if(multicastLock == null) {
			WifiManager wm = (WifiManager)getSystemService(Context.WIFI_SERVICE);
			multicastLock = wm.createMulticastLock("Lumicall");
		}
		
		if(!multicastLock.isHeld())
			multicastLock.acquire();
		
		if(socket == null) {
			System.setProperty("java.net.preferIPv4Stack", "true");
			socket = new MulticastSocket(chosenPTTChannel.getPort());
			socket.joinGroup(InetAddress.getByName(chosenPTTChannel.getAddress()));
			socket.setTimeToLive(chosenPTTChannel.getTtl());
			socket.setLoopbackMode(false);
			socket.setSendBufferSize(512);
			socket.setReceiveBufferSize(512);
			
			//AudioManager am = (AudioManager) Receiver.mContext.getSystemService(Context.AUDIO_SERVICE);
			//am.setSpeakerphoneOn(true);
			//am.setMode(AudioManager.MODE_NORMAL);
		}
		
		SessionDescriptor sd = new SessionDescriptor(offers);
		cmap = Codecs.getCodec(sd);
		
		String _key = chosenPTTChannel.getKey();
		if(srtp == null && _key != null && _key.length() == 40) {
			logger.info("Setting up SRTP");
			SRTPKeySpec key = new SRTPKeySpec(_key);
			srtp = new SRTP(new zorg.platform.j2se.PlatformImpl());
			srtp.setKDR(48);
			srtp.setHmacAuthSizeBytes(UserAgent.SUPPORTED_CRYPTO_SUITE_TAG_SIZE);  // FIXME
			srtp.setTxMasterKey(key.getMaster());
			srtp.setTxMasterSalt(key.getSalt());
			srtp.setRxMasterKey(key.getMaster());
			srtp.setRxMasterSalt(key.getSalt());
			// FIXME: PTT conflicts with SRTP replay protection
			// A branch of the ZORG ZRTP library makes it
			// possible to disable replay protection with this SRTP API call
			// but it is not a supported feature at present and PTT won't
			// work without it.
			//srtp.setReplayProtection(false);
			if(srtp.startNewSession() != SRTP.SESSION_OK) {
				throw new RuntimeException("Failed to start SRTP session");
			}
		}
	}
	

	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if(keyCode != KeyEvent.KEYCODE_CAMERA) {
			return super.onKeyDown(keyCode, event);
		}
		startTX();
		return true;
	}
	
	public void startTX() {
		if(chosenPTTChannel == null)
			return;
		if(receiver != null) {
			receiver.halt();
			receiver = null;
		}
		if(sender != null)
			return;
		try {
			prepareMulticastSocket();
			// Start TX
			logger.info("Starting TX with codec: " + cmap.codec.getTitle());
			int frame_size = cmap.codec.frame_size();
			int frame_rate = cmap.codec.samp_rate()/frame_size;
			sender = new RtpStreamSender(
					false,
					cmap,
					frame_rate,
					frame_size,
					socket,
					chosenPTTChannel.getAddress(),
					chosenPTTChannel.getPort(),
					null,srtp, null);
			sender.setSyncAdj(2);
            sender.setDTMFpayloadType(0);
            sender.setForceTX(true);
			sender.start();
		} catch (Exception ex) {
			ex.printStackTrace();
			sender = null;
		}
	}
	
	MulticastSocket socket = null;
	RtpStreamReceiver receiver = null;
	
	/**
	 * Proposed receiver logic:
	 * - start a thread to monitor the group
	 * - when first packet received, start a full receiver
	 * - receiver stops when audio stops for more than 500ms
	 */
	private void startRX() {
		if(chosenPTTChannel == null)
			return;
		try {
			prepareMulticastSocket();
			
			if(sender != null) {
				// Stop TX
				sender.halt();
				sender = null;
			}
			
			if(receiver == null) {
				receiver = new RtpStreamReceiver(socket, cmap, null, srtp, null);
				receiver.start();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		if(keyCode != KeyEvent.KEYCODE_CAMERA) {
			return super.onKeyDown(keyCode, event);
		}
		startRX();
		return true;
	}
	
	private void setDefaultIdentity() {
		// Must use the default SIP identity for SIP-SIP calls
		SharedPreferences sipSettings = getSharedPreferences(Settings.sharedPrefsFile, Context.MODE_PRIVATE);
		long _sipIdentityId = Long.parseLong(sipSettings.getString(Settings.PREF_SIP, "-1"));
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();
		if(_sipIdentityId >= 0)
			chosenIdentity = ds.getSIPIdentity(_sipIdentityId);
		if(chosenIdentity == null) {
			Iterator<SIPIdentity> iterator = ds.getSIPIdentities().iterator();
			if(iterator.hasNext()) {
				chosenIdentity = iterator.next();
			}
		}
		ds.close();
		if(chosenIdentity != null)
			buttonIdentity.setText(chosenIdentity.getUri());
	}
	
	protected Dialog onCreateDialog(int id) {
		Dialog dialog = null;
		switch(id) {
		case DLG_IDENTITY_ID:
			dialog = new AlertDialog.Builder(this)
				.setIcon(R.drawable.icon22)
				.setTitle(R.string.choose_identity)
				.setItems(new CharSequence[] {}, null)
				.setCancelable(true)
				.setOnCancelListener(new OnCancelListener() {
					@Override
					public void onCancel(DialogInterface arg0) {
						dismissDialog(DLG_IDENTITY_ID);
					}
				})
				.create();
			break;
		case DLG_PTT_CHANNEL_ID:
			dialog = new AlertDialog.Builder(this)
				.setIcon(R.drawable.icon22)
				.setTitle(R.string.ptt_channels)
				.setItems(new CharSequence[] {}, null)
				.setCancelable(true)
				.setOnCancelListener(new OnCancelListener() {
					@Override
					public void onCancel(DialogInterface arg0) {
						dismissDialog(DLG_PTT_CHANNEL_ID);
					}
				})
				.create();
			break;
		default:
			return null;
		}
		return dialog;
	}
	
	protected void onPrepareDialog(int id, Dialog dialog, Bundle bundle) {
		if(id != DLG_IDENTITY_ID && id != DLG_PTT_CHANNEL_ID)
			return;
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();
		AlertDialog _dialog = (AlertDialog)dialog;
		ListView lv = _dialog.getListView();
		MyListener l = null;
		MyArrayAdapter a = null;
		switch(id) {
		case DLG_IDENTITY_ID:
			SIPIdentity[] identities = ds.getSIPIdentities().toArray(new SIPIdentity[] {});
			l = new MyListener<SIPIdentity>(identities, DLG_IDENTITY_ID) {
				@Override
				protected void handleChosen(SIPIdentity object) {
					buttonIdentity.setText(object.getUri());
					chosenIdentity = object;
				}
			};
			a = new MyArrayAdapter<SIPIdentity>(this, identities);
			break;
		case DLG_PTT_CHANNEL_ID:
			List<PTTChannel> _pttChannels = ds.getPTTChannels();
			PTTChannel disablePTT = new PTTChannel();
			disablePTT.setAlias(getString(R.string.ptt_channel_disable));
			_pttChannels.add(0, disablePTT);
			PTTChannel[] pttChannels = _pttChannels.toArray(new PTTChannel[] {});
			l = new MyListener<PTTChannel>(pttChannels, DLG_PTT_CHANNEL_ID) {
				@Override
				protected void handleChosen(PTTChannel object) {
					buttonPTTChannel.setText(object.getAlias());
					if(object.getId() >= 0)
						chosenPTTChannel = object;
					else
						chosenPTTChannel = null;
					updatePTTChannel();
				}
			};
			a = new MyArrayAdapter<PTTChannel>(this, pttChannels);
			break;
		default:
			// should never get here
			throw new RuntimeException("Unexpected !!!");
		}
		lv.setAdapter(a);
		lv.setOnItemClickListener(l);
		ds.close();
	}
	
	public class MyArrayAdapter<T extends DBObject> extends ArrayAdapter<T> {
	
		public MyArrayAdapter(Context context, T[] objects) {
			super(context, R.layout.identity_list_item, objects);
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view = (TextView)super.getView(position, convertView, parent);
			T object = getObject(position);
			view.setText(object.getTitleForMenu());
			view.setTextColor(Color.BLACK);
			return view;
		}
		
		protected T getObject(int position) {
			return getItem(position);
		}
		
	}
	
	public abstract class MyListener<T extends DBObject> implements OnItemClickListener {
		T[] objects;
		int dlgId;
		
		public MyListener(T[] objects, int dlgId) {
			this.objects = objects;
			this.dlgId = dlgId;
		}
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,
				long id) {
			handleChosen(objects[position]);
			dismissDialog(dlgId);
		}
		protected abstract void handleChosen(T object);
	}

	public static boolean on(Context context) {
		return PreferenceManager.getDefaultSharedPreferences(context).getBoolean(Settings.PREF_ON, Settings.DEFAULT_ON);
	}
	
	private static class ContextRunnable implements Runnable {
		Context context;
		public ContextRunnable(Context context) {
			this.context = context;
		}
		public void run() {
			Looper.prepare();
			Receiver.engine(context).isRegistered();
		}
	}
	
	private static volatile Thread kickerThread = null;

	public static synchronized void on(Context context,boolean on) {
		Editor edit = PreferenceManager.getDefaultSharedPreferences(context).edit();
		edit.putBoolean(Settings.PREF_ON, on);
		edit.commit();
		// This is wrapped in a thread so it won't block the GUI
		// FIXME: it should be implemented using the Service paradigm
        if (on) {
        	if (kickerThread == null || !kickerThread.isAlive()) {
        		kickerThread = new Thread(new ContextRunnable(context));
        	}
        	kickerThread.start();
        }
	}

	@Override
	public void onResume() {
		super.onResume();
		if (Receiver.call_state != UserAgent.UA_STATE_IDLE) Receiver.moveTop();
		String text = null;
		/* if (Checkin.createButton == 0 || Random.nextInt(Checkin.createButton) != 0)
			text = null;
		else
			text = Integer.parseInt(Build.VERSION.SDK) >= 5?CreateAccount.isPossible(this):null; */
		setDefaultIdentity();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		boolean result = super.onCreateOptionsMenu(menu);

		MenuItem m = menu.add(0, ABOUT_MENU_ITEM, 0, R.string.menu_about);
		m.setIcon(android.R.drawable.ic_menu_info_details);
		m = menu.add(0, EXIT_MENU_ITEM, 0, R.string.menu_exit);
		m.setIcon(android.R.drawable.ic_menu_close_clear_cancel);
		m = menu.add(0, ADD_ACCOUNT_MENU_ITEM, 0, R.string.menu_add_account);
		m.setIcon(android.R.drawable.ic_menu_preferences);
		m = menu.add(0, CONFIGURE_MENU_ITEM, 0, R.string.menu_settings);
		m.setIcon(android.R.drawable.ic_menu_preferences);
						
		return result;
	}

	void call_menu(AutoCompleteTextView view)
	{
		String target = view.getText().toString();
		if (m_AlertDlg != null) 
		{
			m_AlertDlg.cancel();
		}
		if (target.length() == 0)
			m_AlertDlg = new AlertDialog.Builder(this)
				.setMessage(R.string.empty)
				.setTitle(R.string.app_name)
				.setIcon(R.drawable.icon22)
				.setCancelable(true)
				.show();
		else {
			long chosenIdentityId = -1;
			if (chosenIdentity != null) {
				chosenIdentityId = chosenIdentity.getId();
			}
			DialCandidate dc = new DialCandidate("sip", target, "", "Manual", chosenIdentity.getId());
			if (!DialCandidateHelper.call(this, dc)) {
				// ignoring error
			}
		}
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		boolean result = super.onOptionsItemSelected(item);
		Intent intent = null;

		switch (item.getItemId()) {
		case ABOUT_MENU_ITEM:
			if (m_AlertDlg != null) 
			{
				m_AlertDlg.cancel();
			}
			m_AlertDlg = new AlertDialog.Builder(this)
			.setMessage(getString(R.string.about).replace("\\n","\n").replace("${VERSION}", getVersion(this)))
			.setTitle(getString(R.string.menu_about))
			.setIcon(R.drawable.icon22)
			.setCancelable(true)
			.show();
			break;
			
		case EXIT_MENU_ITEM: 
			on(this,false);
			Receiver.pos(true);
			Receiver.engine(this).halt();
			Receiver.mSipdroidEngine = null;
			Receiver.reRegister(0);
			stopService(new Intent(this,RegisterService.class));
			finish();
			break;
			
		case ADD_ACCOUNT_MENU_ITEM:
			try {
				intent = new Intent(this, org.lumicall.android.sip.RegisterOtherAccount.class);
				startActivity(intent);
			} catch (ActivityNotFoundException e) {
			}
			break;

		case CONFIGURE_MENU_ITEM: {
			try {
				intent = new Intent(this, org.sipdroid.sipua.ui.Settings.class);
				startActivity(intent);
			} catch (ActivityNotFoundException e) {
			}
		}
			break;
		}

		return result;
	}
	
	public static String getVersion() {
		return getVersion(Receiver.mContext);
	}
	
	public static String getVersion(Context context) {
		final String unknown = "Unknown";
		
		if (context == null) {
			return unknown;
		}
		
		try {
	    	String ret = context.getPackageManager()
			   .getPackageInfo(context.getPackageName(), 0)
			   .versionName;
	    	if (ret.contains(" + "))
	    		ret = ret.substring(0,ret.indexOf(" + "))+"b";
	    	return ret;
		} catch(NameNotFoundException ex) {}
		
		return unknown;		
	}

	@Override
	public void onDismiss(DialogInterface dialog) {
		onResume();
	}
}
