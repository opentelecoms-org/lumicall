package org.lumicall.android.db;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.preferences.PreferenceField;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

/**
 * A UserMessage object represents and persists a message body,
 * such as a text message, transmitted through the SIP MESSAGE
 * method.
 */
public class UserMessage extends DBObject {
	
	private final static String DB_TABLE = "UserMessage";
	private final static String COLUMN_ORIGIN_LOCAL = "origin_local";
	private final static String COLUMN_MESSAGE_TIMESTAMP = "sent_ts";
	private final static String COLUMN_RECEIVED_TIMESTAMP = "received_ts";
	private final static String COLUMN_SENDER_NAME = "sender_name";
	private final static String COLUMN_SENDER_URI = "sender_uri";
	private final static String COLUMN_SENDER_CONTACT = "sender_contact";
	private final static String COLUMN_RECIPIENT_NAME = "recipient_name";
	private final static String COLUMN_RECIPIENT_URI = "recipient_uri";
	private final static String COLUMN_RECIPIENT_CONTACT = "recipient_contact";
	private final static String COLUMN_SUBJECT = "subject";
	private final static String COLUMN_CONTENT_TYPE = "content_type";
	private final static String COLUMN_BODY = "body";
	
	private final static String[] ALL_COLUMNS = new String[] {
		COLUMN_ID,
		COLUMN_ORIGIN_LOCAL,
		COLUMN_MESSAGE_TIMESTAMP,
		COLUMN_RECEIVED_TIMESTAMP,
		COLUMN_SENDER_NAME,
		COLUMN_SENDER_URI,
		COLUMN_SENDER_CONTACT,
		COLUMN_RECIPIENT_NAME,
		COLUMN_RECIPIENT_URI,
		COLUMN_RECIPIENT_CONTACT,
		COLUMN_SUBJECT,
		COLUMN_CONTENT_TYPE,
		COLUMN_BODY
	};
	
	private final static String CREATE_TABLE =
			"CREATE TABLE " + DB_TABLE + " (" +
			COLUMN_ID + " integer primary key autoincrement, " +
			COLUMN_ORIGIN_LOCAL + " int not null, " +
			COLUMN_MESSAGE_TIMESTAMP + " integer not null, " +
			COLUMN_RECEIVED_TIMESTAMP + " integer not null, " +
			COLUMN_SENDER_NAME + " text, " +
			COLUMN_SENDER_URI + " text not null, " +
			COLUMN_SENDER_CONTACT + " text, " +
			COLUMN_RECIPIENT_NAME + " text, " +
			COLUMN_RECIPIENT_URI + " text not null, " +
			COLUMN_RECIPIENT_CONTACT + " text, " +
			COLUMN_SUBJECT + " text, " +
			COLUMN_CONTENT_TYPE + " text not null, " +
			COLUMN_BODY + " text not null);";

	boolean originLocal = false;
	long messageTimestamp = 0;
	long receivedTimestamp = 0;
	String senderName = null;
	String senderUri = null;
	String senderContact = null;
	String recipientName = null;
	String recipientUri = null;
	String recipientContact = null;
	String subject = null;
	String contentType = null;
	String body = null;
	
	public static void onCreate(SQLiteDatabase db) {	
		db.execSQL(CREATE_TABLE);
	}
	
	public static void onUpgrade(SQLiteDatabase db, int oldVersion,
			int newVersion) {
		if(oldVersion < 7 && newVersion >= 7) {
			onCreate(db);
		}
	}
	
	public UserMessage() {
		// do nothing
	}
	
	public static List<UserMessage> loadFromDatabase(SQLiteDatabase db) {
		Vector<UserMessage> v = new Vector<UserMessage>();
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, null,
				null, null, null, null);
		cursor.moveToFirst();
		while(!cursor.isAfterLast()) {
			v.add(fromCursor(cursor));
			cursor.moveToNext();
		}
		cursor.close();
		return v;
	}
	
	public static UserMessage loadFromDatabase(SQLiteDatabase db, long id) {
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, COLUMN_ID + " = " + id,
				null, null, null, null);
		cursor.moveToFirst();
		UserMessage req = null;
		if(!cursor.isAfterLast())
			 req = fromCursor(cursor);
		cursor.close();
		return req;
	}
	
	private static UserMessage fromCursor(Cursor cursor) {
		
		UserMessage req = new UserMessage();
		
		int i = 0;
		
		req.setId(cursor.getLong(i++));
		req.setOriginLocal(fromBoolean(cursor.getInt(i++)));
		req.setMessageTimestamp(cursor.getLong(i++));
		req.setReceivedTimestamp(cursor.getLong(i++));
		req.setSenderName(cursor.getString(i++));
		req.setSenderUri(cursor.getString(i++));
		req.setSenderContact(cursor.getString(i++));
		req.setRecipientName(cursor.getString(i++));
		req.setRecipientUri(cursor.getString(i++));
		req.setRecipientContact(cursor.getString(i++));
		req.setSubject(cursor.getString(i++));
		req.setContentType(cursor.getString(i++));
		req.setBody(cursor.getString(i++));

		return req;
	}

	public final static String USER_MESSAGE_ID = "userMessageId";
	
	@Override
	protected String getTableName() {
		return DB_TABLE;
	}
	
	@Override
	protected void putValues(ContentValues values) {
		values.put(COLUMN_ORIGIN_LOCAL, toBoolean(isOriginLocal()));
		values.put(COLUMN_MESSAGE_TIMESTAMP, getMessageTimestamp());
		values.put(COLUMN_RECEIVED_TIMESTAMP, getReceivedTimestamp());
		values.put(COLUMN_SENDER_NAME, getSenderName());
		values.put(COLUMN_SENDER_URI, getSenderUri());
		values.put(COLUMN_SENDER_CONTACT, getSenderContact());
		values.put(COLUMN_RECIPIENT_NAME, getRecipientName());
		values.put(COLUMN_RECIPIENT_URI, getRecipientUri());
		values.put(COLUMN_RECIPIENT_CONTACT, getRecipientContact());
		values.put(COLUMN_SUBJECT, getSubject());
		values.put(COLUMN_CONTENT_TYPE, getContentType());
		values.put(COLUMN_BODY, getBody());
	}

	public boolean isOriginLocal() {
		return originLocal;
	}

	public void setOriginLocal(boolean originLocal) {
		this.originLocal = originLocal;
	}

	public long getMessageTimestamp() {
		return messageTimestamp;
	}

	public void setMessageTimestamp(long messageTimestamp) {
		this.messageTimestamp = messageTimestamp;
	}

	public long getReceivedTimestamp() {
		return receivedTimestamp;
	}

	public void setReceivedTimestamp(long receivedTimestamp) {
		this.receivedTimestamp = receivedTimestamp;
	}

	public String getSenderName() {
		return senderName;
	}

	public void setSenderName(String senderName) {
		this.senderName = senderName;
	}

	public String getSenderUri() {
		return senderUri;
	}

	public void setSenderUri(String senderUri) {
		this.senderUri = senderUri;
	}

	public String getSenderContact() {
		return senderContact;
	}

	public void setSenderContact(String senderContact) {
		this.senderContact = senderContact;
	}

	public String getRecipientName() {
		return recipientName;
	}

	public void setRecipientName(String recipientName) {
		this.recipientName = recipientName;
	}

	public String getRecipientUri() {
		return recipientUri;
	}

	public void setRecipientUri(String recipientUri) {
		this.recipientUri = recipientUri;
	}

	public String getRecipientContact() {
		return recipientContact;
	}

	public void setRecipientContact(String recipientContact) {
		this.recipientContact = recipientContact;
	}

	public String getSubject() {
		return subject;
	}

	public void setSubject(String subject) {
		this.subject = subject;
	}

	public String getContentType() {
		return contentType;
	}

	public void setContentType(String contentType) {
		this.contentType = contentType;
	}

	public String getBody() {
		return body;
	}

	public void setBody(String body) {
		this.body = body;
	}

	@Override
	public String getTitleForMenu() {
		return getId() + "";
	}

	@Override
	public String getKeyForIntent() {
		return USER_MESSAGE_ID;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + ((body == null) ? 0 : body.hashCode());
		result = prime * result
				+ ((contentType == null) ? 0 : contentType.hashCode());
		result = prime * result
				+ (int) (messageTimestamp ^ (messageTimestamp >>> 32));
		result = prime * result + (originLocal ? 1231 : 1237);
		result = prime * result
				+ (int) (receivedTimestamp ^ (receivedTimestamp >>> 32));
		result = prime
				* result
				+ ((recipientContact == null) ? 0 : recipientContact.hashCode());
		result = prime * result
				+ ((recipientName == null) ? 0 : recipientName.hashCode());
		result = prime * result
				+ ((recipientUri == null) ? 0 : recipientUri.hashCode());
		result = prime * result
				+ ((senderContact == null) ? 0 : senderContact.hashCode());
		result = prime * result
				+ ((senderName == null) ? 0 : senderName.hashCode());
		result = prime * result
				+ ((senderUri == null) ? 0 : senderUri.hashCode());
		result = prime * result + ((subject == null) ? 0 : subject.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (!super.equals(obj))
			return false;
		if (getClass() != obj.getClass())
			return false;
		UserMessage other = (UserMessage) obj;
		if (body == null) {
			if (other.body != null)
				return false;
		} else if (!body.equals(other.body))
			return false;
		if (contentType == null) {
			if (other.contentType != null)
				return false;
		} else if (!contentType.equals(other.contentType))
			return false;
		if (messageTimestamp != other.messageTimestamp)
			return false;
		if (originLocal != other.originLocal)
			return false;
		if (receivedTimestamp != other.receivedTimestamp)
			return false;
		if (recipientContact == null) {
			if (other.recipientContact != null)
				return false;
		} else if (!recipientContact.equals(other.recipientContact))
			return false;
		if (recipientName == null) {
			if (other.recipientName != null)
				return false;
		} else if (!recipientName.equals(other.recipientName))
			return false;
		if (recipientUri == null) {
			if (other.recipientUri != null)
				return false;
		} else if (!recipientUri.equals(other.recipientUri))
			return false;
		if (senderContact == null) {
			if (other.senderContact != null)
				return false;
		} else if (!senderContact.equals(other.senderContact))
			return false;
		if (senderName == null) {
			if (other.senderName != null)
				return false;
		} else if (!senderName.equals(other.senderName))
			return false;
		if (senderUri == null) {
			if (other.senderUri != null)
				return false;
		} else if (!senderUri.equals(other.senderUri))
			return false;
		if (subject == null) {
			if (other.subject != null)
				return false;
		} else if (!subject.equals(other.subject))
			return false;
		return true;
	}

}
