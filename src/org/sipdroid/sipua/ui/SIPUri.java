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

import org.lumicall.android.R;
import org.lumicall.android.sip.DialCandidate;
import org.lumicall.android.sip.SIPCarrierCandidateHarvester;
import org.sipdroid.sipua.SipdroidEngine;

import android.app.Activity;
import android.app.AlertDialog;
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
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class SIPUri extends Activity {

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
			if(dc.getScheme().equals("tel"))
				view.setTextColor(GSM_COLOUR);
			else if(dc.getSource().equals(SIPCarrierCandidateHarvester.SOURCE_INFO))
				view.setTextColor(VOIP_CARRIER_COLOUR);
			else
				view.setTextColor(SIP_COLOUR);
			return view;
		}
		
		protected DialCandidate getDialCandidate(int position) {
			return getItem(position);
		}
		
	}
	
	public class MyListener implements DialogInterface.OnClickListener {
		DialCandidate[] candidates;
		DialCandidate candidate;
		public MyListener(DialCandidate[] candidates) {
			this.candidates = candidates;
			this.candidate = null;
		}
		@Override
		public void onClick(DialogInterface dialog, int which) {
			candidate = candidates[which];
			if(candidate.getScheme().equals("sip")) {
				Receiver.engine(SIPUri.this).call(candidate, true);
			} else if(candidate.getScheme().equals("tel")) {
				PSTN.callPSTN2("tel:" + candidate.getAddress() + "?bypass-lumicall");
			}
			finish();
		}
		public DialCandidate getCandidate() {
			return candidate;
		}
	}
	
	/* (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    	if (Receiver.mContext == null) Receiver.mContext = this;

		requestWindowFeature(Window.FEATURE_NO_TITLE);

		Sipdroid.on(this,true);
		Uri uri = getIntent().getData();
		if(getIntent().getParcelableArrayExtra("dialCandidates") != null) {
			Parcelable[] _candidates = getIntent().getParcelableArrayExtra("dialCandidates");
			DialCandidate[] candidates = new DialCandidate[_candidates.length];
			for(int i = 0; i < _candidates.length; i++) {
				candidates[i] = (DialCandidate)_candidates[i];
			}
			MyListener l = new MyListener(candidates);
			new AlertDialog.Builder(this)
				.setIcon(R.drawable.icon22)
				.setTitle(R.string.choose_route)
				.setAdapter(new MyArrayAdapter(this, candidates), l)
				.setOnCancelListener(new OnCancelListener() {
					public void onCancel(DialogInterface dialog) {	finish();	} })
				.show();
			return;
		}
		
		String target;
		if (uri.getScheme().equals("sip") || uri.getScheme().equals(Settings.URI_SCHEME))
			target = uri.getSchemeSpecificPart();
		else {
			if (uri.getAuthority().equals("aim") ||
					uri.getAuthority().equals("yahoo") ||
					uri.getAuthority().equals("icq") ||
					uri.getAuthority().equals("gtalk") ||
					uri.getAuthority().equals("msn"))
				target = uri.getLastPathSegment().replaceAll("@","_at_") + "@" + uri.getAuthority() + ".gtalk2voip.com";
			else if (uri.getAuthority().equals("skype"))
				target = uri.getLastPathSegment() + "@" + uri.getAuthority();
			else
				target = uri.getLastPathSegment();
		}
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
	        finish();
	    }
	 
}
