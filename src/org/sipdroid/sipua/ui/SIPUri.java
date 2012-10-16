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
import org.lumicall.android.sip.DialCandidate;
import org.lumicall.android.sip.SIPCarrierCandidateHarvester;
import org.sipdroid.sipua.SipdroidEngine;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Parcelable;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class SIPUri extends Activity {
	
	private Logger logger = Logger.getLogger(getClass().getCanonicalName());

	void call(String target) {
		if (!Receiver.engine(this).call(target,true)) {
			new AlertDialog.Builder(this)
			.setMessage(R.string.notfast)
			.setTitle(R.string.app_name)
			.setIcon(R.drawable.icon22)
			.setCancelable(true)
			.setOnCancelListener(new OnCancelListener() {
				public void onCancel(DialogInterface dialog) {
					finish();
				}
			})
			.show();
		} else
			finish();
	}
	
	public class MyArrayAdapter extends ArrayAdapter<DialCandidate> {
		
		final int SIP_COLOUR = Color.rgb(50, 205, 50);
		final static int VOIP_CARRIER_COLOUR = Color.BLUE;
		final static int GSM_COLOUR = Color.RED;

		public MyArrayAdapter(Context context, DialCandidate[] objects) {
			super(context, R.layout.candidate_list_item, objects);
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
			return view;
		}
		
		protected DialCandidate getDialCandidate(int position) {
			return getItem(position);
		}
		
	}
	
	public class MyListener implements OnItemClickListener {
		DialCandidate[] candidates;
		DialCandidate candidate;
		public MyListener(DialCandidate[] candidates) {
			this.candidates = candidates;
			this.candidate = null;
		}
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,
				long id) {
			candidate = candidates[position];
			if(!candidate.call(SIPUri.this)) {
				// ignoring error
			}
			finish();
		}
		public DialCandidate getCandidate() {
			return candidate;
		}
	}
	
	protected DialCandidate[] getDialCandidates(Bundle bundle) {
		Parcelable[] _candidates = bundle.getParcelableArray("dialCandidates");
		DialCandidate[] candidates = new DialCandidate[_candidates.length];
		for(int i = 0; i < _candidates.length; i++) {
			candidates[i] = (DialCandidate)_candidates[i];
		}
		return candidates;
	}
	
	protected void onPrepareDialog(int id, Dialog dialog, Bundle bundle) {
		if(id != 0)
			return;
		DialCandidate[] candidates = getDialCandidates(bundle);
		MyListener l = new MyListener(candidates);
		AlertDialog _dialog = (AlertDialog)dialog;
		_dialog.getListView().setAdapter(new MyArrayAdapter(this, candidates));
		_dialog.getListView().setOnItemClickListener(l);
	}
	
	protected Dialog onCreateDialog(int id) {
		if(id != 0)
			return null;
		Dialog dialog = new AlertDialog.Builder(this)
			.setIcon(R.drawable.icon22)
			.setTitle(R.string.choose_route)
			.setOnCancelListener(new OnCancelListener() {
				public void onCancel(DialogInterface dialog) {	finish();	} })
			.setItems(new CharSequence[] {}, null)
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
		Parcelable[] _candidates = getIntent().getParcelableArrayExtra("dialCandidates");
		if(_candidates != null) {
			Bundle bundle = new Bundle();
			bundle.putParcelableArray("dialCandidates", _candidates);
			showDialog(0, bundle);
			return;
		}
		
		String target = null;
		if (uri.getScheme().equals("sip") || uri.getScheme().equals(Settings.URI_SCHEME)) {
			target = uri.getSchemeSpecificPart();
			logger.fine("found a SIP URI: " + target);
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
	 
}
