/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
 * Copyright (C) 2009 Nominet UK and contributed to
 * the Sipdroid Open Source Project
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

import java.util.List;
import java.util.logging.Logger;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.sip.DialCandidate;
import org.lumicall.android.sip.DialCandidateHarvester;
import org.lumicall.android.sip.DialCandidateListener;
import org.lumicall.android.sip.HarvestDirector;
import org.lumicall.android.sip.SIPCarrierCandidateHarvester;
import org.sipdroid.sipua.Constants;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnCancelListener;
import android.content.SharedPreferences.Editor;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

public class SIPUri extends Activity implements DialCandidateListener {
	
	private Logger logger = Logger.getLogger(getClass().getCanonicalName());
	private MyArrayAdapter adapter;
	HarvestDirector hd = null;
	FrameLayout frameView = null;
	AlertDialog _dialog = null;
	LinearLayout harvestStatus = null;

	void call(String target) {
		String _domain = target.substring(target.indexOf('@') + 1);
		LumicallDataSource ds = new LumicallDataSource(this);
		ds.open();

		List<SIPIdentity> sipIdentities = ds.getSIPIdentities();

		SIPIdentity sipIdentity = null;
		for(SIPIdentity s : sipIdentities) {
			String uri = s.getUri();
			String domain = uri.substring(uri.indexOf('@') + 1);
			if(domain.equals(_domain))
			{
				sipIdentity = s;
				logger.fine("matched domain: " + domain + ", using identity: " + s.getUri());
			}
		}

		ds.close();
		DialCandidate dc = new DialCandidate("sip", target, "", "Manual dial", sipIdentity);
		if(!dc.call(this)) {
			// ignoring error
			logger.severe("DialCandidate failed to place call");
		} else
			finish();
	}
	
	public class MyArrayAdapter extends ArrayAdapter<DialCandidate> {
		
		final int SIP_COLOUR = Color.rgb(50, 205, 50);
		final static int VOIP_CARRIER_COLOUR = Color.BLUE;
		final static int GSM_COLOUR = Color.RED;
		final static int BACKGROUND_COLOUR = Color.WHITE;

		public MyArrayAdapter(Context context) {
			super(context, R.layout.candidate_list_item);
			
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view = (TextView)super.getView(position, convertView, parent);
			// Set the colour
			DialCandidate dc = getDialCandidate(position);
			int routeColour = SIP_COLOUR;
			if(dc.getScheme().equals("tel"))
				routeColour = GSM_COLOUR;
			else if(dc.getSource().equals(SIPCarrierCandidateHarvester.SOURCE_INFO))
				routeColour = VOIP_CARRIER_COLOUR;
			// view.setBackgroundColor(routeColour);
			// view.setTextColor(Color.WHITE);
			view.setTextColor(routeColour);
			view.setBackgroundColor(BACKGROUND_COLOUR);
			return view;
		}
		
		protected DialCandidate getDialCandidate(int position) {
			return getItem(position);
		}
	}
	
	private void dialEntry(int position) {
		DialCandidate candidate = adapter.getItem(position);
		if(candidate.call(SIPUri.this)) {
			finish();
		} else {
			logger.severe("error occurred while trying to start call");
		}		
	}
		
	@Override
	protected void onPrepareDialog(int id, Dialog dialog, Bundle bundle) {
		if(id != 0)
			return;
		adapter = new MyArrayAdapter(this);
		
		_dialog = (AlertDialog)dialog;
		
		LayoutInflater inflater = getLayoutInflater();
		//final FrameLayout frameView = new FrameLayout(this);
		View layout = inflater.inflate(R.layout.dialog_popup_view, frameView);
		_dialog.setView(frameView);
		
		ListView routeList = (ListView) layout.findViewById(R.id.route_list);
		routeList.setAdapter(adapter);
		
		routeList.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position,
					long id) {
				dialEntry(position);
			}
		});
		
		String number = bundle.getString("number");
		String e164Number = bundle.getString("e164Number");
		
		harvestStatus = (LinearLayout) layout.findViewById(R.id.route_search_status);
		ProgressBar progress = new ProgressBar(this);
		progress.setIndeterminate(true);
		harvestStatus.removeAllViews();
		harvestStatus.addView(progress);
		
		CheckBox alwaysShow = (CheckBox)layout.findViewById(R.id.route_popup_enable);
		boolean dialingIntegration =
				PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
						Settings.PREF_DIALING_INTEGRATION, Settings.DEFAULT_DIALING_INTEGRATION);
		alwaysShow.setChecked(dialingIntegration);
		alwaysShow.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(SIPUri.this);
				Editor edit = sp.edit();
				edit.putBoolean(Settings.PREF_DIALING_INTEGRATION, isChecked);
				edit.commit();
			}
		});
		
		hd = new HarvestDirector(this);
		hd.addListener(this);
		hd.getCandidatesForNumber(number, e164Number);
	}
	
	@Override
	public void onDialCandidate(DialCandidateHarvester h, final DialCandidate dc) {
		this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				adapter.add(dc);
			}
		});
	}
	
	@Override
	public void onHarvestCompletion(DialCandidateHarvester h, final int resultCount) {
		h.removeListener(this);
		if(h == hd) {
			hd = null;
		}
		this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Resources res = getResources();
				String status = String.format(res.getString(R.string.dial_popup_done), resultCount);
				harvestStatus.removeAllViews();
				TextView statusView = new TextView(SIPUri.this);
				statusView.setText(status);
				statusView.setGravity(Gravity.CENTER | Gravity.BOTTOM);
				harvestStatus.addView(statusView);
				if(resultCount == 1) {
					// Only one result - dial automatically
					dialEntry(0);
				}
			}
		});
	}

	protected Dialog onCreateDialog(int id) {
		if(id != 0)
			return null;
		frameView = new FrameLayout(this);
		Dialog dialog = new AlertDialog.Builder(this)
			.setIcon(R.drawable.icon22)
			.setTitle(R.string.choose_route)
			.setOnCancelListener(new OnCancelListener() {
				public void onCancel(DialogInterface dialog) {	finish();	} })
			.setView(frameView)
			.create();
		return dialog;
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    	if (Receiver.mContext == null) Receiver.mContext = this;

		requestWindowFeature(Window.FEATURE_NO_TITLE);

		Uri uri = getIntent().getData();
		String number = getIntent().getStringExtra("number");
		if(number != null) {
			Bundle bundle = new Bundle();
			bundle.putString("number", number);
			String e164Number = getIntent().getStringExtra("e164Number");
			bundle.putString("e164Number", e164Number);
			showDialog(0, bundle);
			return;
		}
		
		String target = null;
		if (uri.getScheme().equals("sip") || uri.getScheme().equals(Settings.URI_SCHEME) ||
				uri.getScheme().equals(Constants.URI_PREFIX)) {
			target = uri.getSchemeSpecificPart();
			boolean logUriHack = PreferenceManager.getDefaultSharedPreferences(this).getBoolean(Settings.PREF_LOG_URI_HACK, Settings.DEFAULT_LOG_URI_HACK);
    		if (logUriHack) {
    			int i = target.indexOf(Constants.SUBSTITUTE_AT);
    			if (i >= 0) {
    				logger.fine("found a substitute @, putting back regular @");
    				String userPart = target.substring(0, i);
    				String finalPart = target.substring(i+1);
    				target = userPart + '@' + finalPart;
    			}
    		}
			logger.fine("found a SIP URI: " + target);
		} else if(uri.getScheme().equals("tel") && !uri.getSchemeSpecificPart().contains("@")) {
			/*final Intent intent = new Intent(getIntent());
			intent.setAction("android.intent.action.CALL_PHONE");
			intent.setComponent(new ComponentName("com.android.phone", ""));*/
			Intent intent = new Intent(Intent.ACTION_CALL, uri);
			logger.info("Sending a numeric dialing attempt to default dialer: " + uri);
			startActivity(intent);
			finish();
		} else {
			if(uri.getAuthority() != null) {
				if (uri.getAuthority().equals("aim") ||
					uri.getAuthority().equals("yahoo") ||
					uri.getAuthority().equals("icq") ||
					uri.getAuthority().equals("gtalk") ||
					uri.getAuthority().equals("msn")) {
					target = uri.getLastPathSegment().replaceAll("@","_at_") + "@" + uri.getAuthority() + ".gtalk2voip.com";
					logger.fine("found a proprietary authority" + target);
				} else if (uri.getAuthority().equals("skype")) {
					target = uri.getLastPathSegment() + "@" + uri.getAuthority();
					logger.fine("found a proprietary (Skype) URI" + target);
				}
			}
		}
		if(target == null) {
			if(uri.getLastPathSegment() != null)
				target = uri.getLastPathSegment();
			else
				target = uri.getSchemeSpecificPart();
			logger.fine("found a URI that is not explicitly recognised: " + target);
		}
		
		// This call appears to try and register synchronously which
		// causes the process to block during the dial phase
		// Registering should be done async if possible
		// If calling depends on registration, then the caller window
		// should show the status and a progress meter
		Sipdroid.on(this,true);

		if (!Sipdroid.release) Log.v("SIPUri", "sip uri: " + target);
		if (!target.contains("@") && PreferenceManager.getDefaultSharedPreferences(this).getString(Settings.PREF_PREF, Settings.DEFAULT_PREF).equals(Settings.VAL_PREF_ASK)) {
			final String t = target;
			String items[] = {getString(R.string.pstn_name)};
			for (int p = 0; p < Receiver.engine(Receiver.mContext).getLineCount(); p++)
				if (Receiver.isFast(p)) {
					items = new String[2];
					items[0] = getString(R.string.app_name);
					items[1] = getString(R.string.pstn_name);
					break;
				}
			final String fitems[] = items;
			new AlertDialog.Builder(this)
			.setIcon(R.drawable.icon22)
			.setTitle(target)
            .setItems(items, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                    	if (fitems[whichButton].equals(getString(R.string.app_name)))
                    		call(t);
                    	else {
                			PSTN.callPSTN("sip:"+t);
                			finish();
                    	}
                    }
                })
			.setOnCancelListener(new OnCancelListener() {
				public void onCancel(DialogInterface dialog) {
					finish();
				}
			})
			.show();
		} else
			call(target); 
	}
	
	    @Override
	    public void onPause() {
	        super.onPause();
	        //finish();
	    }
	    
	    @Override
	    public void onDestroy() {
	    	if(hd != null) {
	    		hd.removeListener(this);
	    		hd = null;
	    	}
	    	super.onDestroy();
	    }
	 
}
