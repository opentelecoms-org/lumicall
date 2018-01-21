/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
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

import java.util.logging.Logger;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;

public class PSTN extends Activity {

	public static final String BYPASS_LUMICALL = "bypassLumicall";
	
	static Logger logger = Logger.getLogger(PSTN.class.getCanonicalName());

	static void callPSTN(String uri) {
		String number;
		
		if (uri.indexOf(":") >= 0) {
			number = uri.substring(uri.indexOf(":")+1);
			if (!number.equals("")) {
		        Intent intent = new Intent(Intent.ACTION_CALL,
		                Uri.fromParts("tel", Uri.decode(number)+
		                		(!PreferenceManager.getDefaultSharedPreferences(Receiver.mContext).getString(Settings.PREF_PREF, Settings.DEFAULT_PREF).equals(Settings.VAL_PREF_PSTN) ? "+" : ""), null));
		        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		        Receiver.mContext.startActivity(intent);
			}
		}
	}
	
	public static void callPSTN2(String uri) {
		String number;
		
		logger.fine("uri = " + uri);
		if (uri.indexOf(":") >= 0) {
			number = uri.substring(uri.indexOf(":")+1);
			if (!number.equals("")) {
				Context context = Receiver.mContext;
				if(context != null) {
					Intent intent = new Intent(Intent.ACTION_CALL,
		                Uri.fromParts("tel", Uri.decode(number), BYPASS_LUMICALL));
					intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					intent.putExtra("com.android.phone.extra.GATEWAY_PROVIDER_PACKAGE", "org.lumicall");
					intent.putExtra("com.android.phone.extra.GATEWAY_URI", Uri.fromParts("tel", Uri.decode(number), BYPASS_LUMICALL).toString());
					context.startActivity(intent);
				} else {
					// FIXME: we may need to display a popup for the user
					logger.warning("callPSTN2: can't get context to sent intent, Receiver.mContext == null");
				}
			}
			else
			{
				logger.warning("number is empty");
			}
		}
		else
		{
			logger.warning("no colon in URI: " + uri);
		}
	}
	
	@Override
	public void onCreate(Bundle saved) {
		super.onCreate(saved);
		Intent intent;
		Uri uri;
    	if (Receiver.mContext == null) Receiver.mContext = this;
		if ((intent = getIntent()) != null
			&& (uri = intent.getData()) != null)
				callPSTN(uri.toString());
		finish();
	}
}
