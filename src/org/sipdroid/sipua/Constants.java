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
package org.sipdroid.sipua;

import org.lumicall.android.R;

public class Constants {
	public class TransportConstants {
		public final static byte UND_SOCK = 0;
		public final static byte TCP_SOCK = 1;
		public final static byte UDP_SOCK = 2;

	}

	// Also defined in the Android manifest
	public static final String URI_PREFIX = "lumicall";
	
	// This is an alternative @ symbol from Unicode
	// used as part of the workaround for Android bug #58097
	public static final char SUBSTITUTE_AT = '\uFE6B';
	
	// These are SIP URI prefixes that we may encounter including
	// both official prefixes and app-specific prefixes
	public static final String[] POSSIBLE_PREFIXES = {
			URI_PREFIX,   // ourselves
			"sip",
			"sips",
			"sipdroid",
			"csip"        // CSipSimple
		};
}
