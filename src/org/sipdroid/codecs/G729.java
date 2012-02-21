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
package org.sipdroid.codecs;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Sipdroid;
import org.zoolu.tools.Base64;

import android.content.Context;
import android.telephony.TelephonyManager;

class G729 extends CodecBase implements Codec {

	G729() {
		CODEC_NAME = "G729";
		CODEC_USER_NAME = "G729";
		CODEC_DESCRIPTION = "8kbit";
		CODEC_NUMBER = 18;
		CODEC_DEFAULT_SETTING = "always";
		CODEC_FRAME_SIZE = 80 * 2;          // FIXME - hack - send two frames in each packet
		CODEC_FRAMES_PER_PACKET = 2;
		super.update();
	}


	void load() {
		try {
			System.loadLibrary("g729_jni");
			super.load();
		} catch (Throwable e) {
			if (!Sipdroid.release) e.printStackTrace();
		}
    
	}  
 
	public native int open();
	public native int decode(byte encoded[], short lin[], int size);
	public native int encode(short lin[], int offset, byte encoded[], int size);
	public native void close();

	public void init() {
		if(!isLoaded())
			load();
		if (isLoaded())
			open();
	}
	
	final static String hexChars = "0123456789abcdef";
	public String byteToHexString(byte[] buffer, int offset, int length) {
		if(buffer == null)
			return "<null buffer>";
		// FIXME - performance
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < length; i++) {
			byte b = buffer[offset + i];
			sb.append(hexChars.charAt((b & 0xf0) >> 4));
			sb.append(hexChars.charAt(b&0xf));
		}
		return sb.toString();
	}
	
	String[] LICENSED_USERS = new String[] {
		"b0f5a5550dced1f6e0fd6899ade48b4f"
	};
	
	public boolean isLicensed() {
		TelephonyManager telephonyManager = (TelephonyManager)Receiver.mContext.getSystemService(Context.TELEPHONY_SERVICE);
		String devId = telephonyManager.getDeviceId();
		String key = devId.substring(devId.length() - 6);
		try {
			MessageDigest md = MessageDigest.getInstance("MD5");
			md.update(key.getBytes());
			byte[] _hash = md.digest();
			String hash = byteToHexString(_hash, 0, _hash.length);
			for(String u : LICENSED_USERS) {
				if(u.toLowerCase().equals(hash.toLowerCase()))
					return true;
			}
		} catch (NoSuchAlgorithmException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return false;
	}
}
