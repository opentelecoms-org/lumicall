/*
 * Copyright 2009 Nominet UK
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lumicall.android.sip;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;

public class ENUMUtil {

	/* Log tag */
	// static private final String TAG = "ENUMUtil";

	/* Class variables */
	static private boolean mMobile = true;
	static private boolean mOnline = false;

	static public synchronized boolean checkConnectivity(Context context) {

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		// mMobile = prefs.getBoolean(ENUMPrefs.ENUM_PREF_MOBILE, false);
		mOnline = false;

		ConnectivityManager mCmgr = (ConnectivityManager) context
				.getSystemService(Service.CONNECTIVITY_SERVICE);

		NetworkInfo ni = mCmgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
		if (ni != null && ni.isConnected()) { /* wifi is on */
			mOnline = true;
		} else {
			if (mMobile) {
				/* if mobile is active and EDGE or better, we're good */
				ni = mCmgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
				if (ni != null && ni.isConnected()) {
					mOnline = (ni.getSubtype() >= TelephonyManager.NETWORK_TYPE_EDGE);
				}
			}
		}
		return mOnline;
	}

	static public boolean updateNotification(Context context) {

		// check the current system status
		checkConnectivity(context);

		// clicking this will open the prefs pane
		// PendingIntent pi = PendingIntent.getActivity(context, 0,
		//		new Intent(context, ENUMPrefs.class), 0);

/*		NotificationManager mNmgr = (NotificationManager) context
				.getSystemService(Service.NOTIFICATION_SERVICE);

		int text, icon;
		if (mOnline) {
			text = R.string.notify_enum_enabled;
			icon = R.drawable.ic_enum_on;
		} else {
			text = R.string.notify_enum_offline;
			icon = R.drawable.ic_enum_offline;
		}

		String str1 = context.getString(text);
		String str2 = context.getString(R.string.notify_summary);

		Notification n = new Notification();
		n.icon = icon;
		n.flags |= Notification.FLAG_ONGOING_EVENT;
		n.when = 0;
		n.setLatestEventInfo(context, str1, str2, pi);
		mNmgr.notify(0, n);  */
		
		return mOnline;
	}
}
