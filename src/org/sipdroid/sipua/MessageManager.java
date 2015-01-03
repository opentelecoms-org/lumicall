package org.sipdroid.sipua;

import java.util.Date;
import java.util.List;
import java.util.logging.Logger;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIPIdentity;
import org.lumicall.android.db.UserMessage;
import org.lumicall.android.sip.DialCandidate;
import org.lumicall.android.sip.MessageIndex;
import org.sipdroid.sipua.ui.Receiver;
import org.sipdroid.sipua.ui.Sipdroid;
import org.zoolu.sip.address.NameAddress;
import org.zoolu.sip.message.Message;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;

/**
 * This is a basic messaging manager that logs messages to an SQLite
 * table.
 *
 * A future implementation should offer the option of integration with the
 * Android platform SMS content provider.  It would still need to fall back
 * to this in-app data store in cases where Lumicall is not the default
 * messaging app.
 */
public class MessageManager implements MessageAgentListener {
	
	private Logger logger = Logger.getLogger(getClass().getCanonicalName());
	
	private Notification notification;
	private NotificationManager nm;
	
	public MessageManager() {
		nm = (NotificationManager) Receiver.mContext.getSystemService(android.content.Context.NOTIFICATION_SERVICE);
	}

	@Override
	public void onMaReceivedMessage(MessageAgent ma, NameAddress sender,
			NameAddress recipient, String subject, String content_type,
			String body, Message message) {
		
		logger.info("incoming message from: " + sender + " to: " + recipient + " size: " + body.length());
		logger.fine("body text: " + body);
		
		UserMessage um = new UserMessage();
		um.setOriginLocal(false);
		um.setReceivedTimestamp(System.currentTimeMillis()/1000);
		if(message.hasDateHeader()) {
			Date messageTimestamp = message.getDateHeader().getDate();
			um.setMessageTimestamp(messageTimestamp.getTime()/1000);
		}
		um.setSenderName(sender.getDisplayName());
		um.setSenderUri(sender.getAddress().toString());
		um.setRecipientName(recipient.getDisplayName());
		um.setRecipientUri(recipient.getAddress().toString());
		um.setSubject(subject);
		um.setContentType(content_type);
		um.setBody(body);
		
		/**
		 * FIXME - make sure this is thread safe, SQLite can be used from
		 * any thread but only one can write.
		 */
		LumicallDataSource ds = new LumicallDataSource(Receiver.mContext);
		ds.open();
		ds.persistUserMessage(um);
		ds.close();
		
		makeNotification(Receiver.mContext, body);
		
		// If the MessageIndex is visible, update it
		Intent intent = new Intent(MessageIndex.MESSAGE_LIST_CHANGE);
		Receiver.mContext.sendBroadcast(intent);
	}
		
	protected void makeNotification(Context ctx, String detail) {
		notification = new Notification(R.drawable.icon22, ctx.getText(R.string.notify_sms_received), new Date().getTime());
		Intent notificationIntent = new Intent(ctx, MessageIndex.class);
		PendingIntent contentIntent = PendingIntent.getActivity(ctx, 0, notificationIntent, 0);
		notification.setLatestEventInfo(ctx, ctx.getText(R.string.notify_sms_received), detail, contentIntent);
		Uri alarmSound = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
		notification.sound = alarmSound;
		notification.flags |= (Notification.FLAG_AUTO_CANCEL | Notification.FLAG_SHOW_LIGHTS);
		notification.ledOnMS = 1;
		notification.ledOffMS = 0;
		notification.ledARGB = 0xff0011ff;
        nm.notify(10, notification);
	}

	@Override
	public void onMaDeliverySuccess(MessageAgent ma, NameAddress recipient,
			String subject, String result) {
		// TODO Auto-generated method stub
		
		// message sending not implemented yet
		
	}

	@Override
	public void onMaDeliveryFailure(MessageAgent ma, NameAddress recipient,
			String subject, String result) {
		// TODO Auto-generated method stub
		
		// message sending not implemented yet
		
	}

}
