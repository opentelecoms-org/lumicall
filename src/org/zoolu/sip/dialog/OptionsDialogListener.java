/*
 * Copyright (C) 2013 Daniel Pocock <daniel@pocock.com.au>
 * Copyright (C) 2005 Luca Veltri - University of Parma - Italy
 * 
 * This file is part of MjSip (http://www.mjsip.org)
 * 
 * MjSip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * MjSip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MjSip; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author(s):
 * Luca Veltri (luca.veltri@unipr.it)
 */

package org.zoolu.sip.dialog;

import org.zoolu.sip.message.Message;
import org.zoolu.sip.address.NameAddress;

/**
 * A OptionsDialogListener listens for OptionsDialog events. It collects
 * all OptionsDialog callback functions.
 */
public interface OptionsDialogListener {
	/**
	 * When a 2xx successfull final response is received for an OPTIONS
	 * transaction.
	 */
	public void onDlgOptionsSuccess(OptionsDialog dialog, int code,
			String reason, Message msg);

	/** When a 300-699 response is received for an OPTIONS transaction. */
	public void onDlgOptionsFailure(OptionsDialog dialog, int code,
			String reason, Message msg);

	/** When OPTIONS transaction expires without a final response. */
	public void onDlgOptionsTimeout(OptionsDialog dialog);

}
