package org.lumicall.android.db;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.preferences.PreferenceField;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class PTTChannel extends DBObject {
	
	private final static String DB_TABLE = "PTTChannel";
	private final static String COLUMN_ALIAS = "alias";
	private final static String COLUMN_ADDRESS = "address";
	private final static String COLUMN_PORT = "port";
	private final static String COLUMN_TTL = "ttl";
	private final static String COLUMN_KEY = "key";
	
	private final static String[] ALL_COLUMNS = new String[] {
		COLUMN_ID,
		COLUMN_ALIAS,
		COLUMN_ADDRESS,
		COLUMN_PORT,
		COLUMN_TTL,
		COLUMN_KEY
	};
	
	private final static String CREATE_TABLE =
			"CREATE TABLE " + DB_TABLE + " (" +
			COLUMN_ID + " integer primary key autoincrement, " +
			COLUMN_ALIAS + " text not null, " +
			COLUMN_ADDRESS + " text not null, " +
			COLUMN_PORT + " int not null, " +
			COLUMN_TTL + " int not null, " +
			COLUMN_KEY + " text);";
	
	String alias = "Test channel";
	String address = "224.0.224.1";
	int port = 5004;
	int ttl = 31;  // 31 = within organisation
	String key = null;  // null = no SRTP, value = SRTP key|salt in base64
	
	public static void onCreate(SQLiteDatabase db) {	
		db.execSQL(CREATE_TABLE);
		makeChannels(db);
	}
	
	public static void onUpgrade(SQLiteDatabase db, int oldVersion,
			int newVersion) {
		if(oldVersion < 4 && newVersion >= 4) {
			onCreate(db);
		}
	}
	
	public PTTChannel() {	
	}
	
	public static List<PTTChannel> loadFromDatabase(SQLiteDatabase db) {
		Vector<PTTChannel> v = new Vector<PTTChannel>();
		
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
	
	public static PTTChannel loadFromDatabase(SQLiteDatabase db, long id) {
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, COLUMN_ID + " = " + id,
				null, null, null, null);
		cursor.moveToFirst();
		PTTChannel sipIdentity = fromCursor(cursor);
		cursor.close();
		return sipIdentity;
	}
	
	private static PTTChannel fromCursor(Cursor cursor) {
		
		PTTChannel pttChannel = new PTTChannel();
		
		int i = 0;
		
		pttChannel.setId(cursor.getLong(i++));
		pttChannel.setAlias(cursor.getString(i++));
		pttChannel.setAddress(cursor.getString(i++));
		pttChannel.setPort(cursor.getInt(i++));
		pttChannel.setTtl(cursor.getInt(i++));
		pttChannel.setKey(cursor.getString(i++));
		
		return pttChannel;
	}
	
	public final static String PTT_CHANNEL_ID = "pttChannelId";
	public String getIdForIntent() {
		return PTT_CHANNEL_ID;
	}
	
	@Override
	protected String getTableName() {
		return DB_TABLE;
	}
	
	@Override
	protected void putValues(ContentValues values) {
		values.put(COLUMN_ALIAS, getAlias());
		values.put(COLUMN_ADDRESS, getAddress());
		values.put(COLUMN_PORT, getPort());
		values.put(COLUMN_TTL, getTtl());
		values.put(COLUMN_KEY, getKey());
	}

	/**
	 * @return the alias
	 */
	@PreferenceField(fieldName="ptt_channel_alias")
	public String getAlias() {
		return alias;
	}

	/**
	 * @param alias the alias to set
	 */
	@PreferenceField(fieldName="ptt_channel_alias")
	public void setAlias(String alias) {
		this.alias = alias;
	}

	/**
	 * @return the address
	 */
	@PreferenceField(fieldName="ptt_channel_address")
	public String getAddress() {
		return address;
	}

	/**
	 * @param address the address to set
	 */
	@PreferenceField(fieldName="ptt_channel_address")
	public void setAddress(String address) {
		this.address = address;
	}

	/**
	 * @return the port
	 */
	@PreferenceField(fieldName="ptt_channel_port")
	public int getPort() {
		return port;
	}

	/**
	 * @param port the port to set
	 */
	@PreferenceField(fieldName="ptt_channel_port")
	public void setPort(int port) {
		this.port = port;
	}

	/**
	 * @return the ttl
	 */
	@PreferenceField(fieldName="ptt_channel_ttl")
	public int getTtl() {
		return ttl;
	}

	/**
	 * @param ttl the ttl to set
	 */
	@PreferenceField(fieldName="ptt_channel_ttl")
	public void setTtl(int ttl) {
		this.ttl = ttl;
	}

	/**
	 * @return the key
	 */
	@PreferenceField(fieldName="ptt_channel_key")
	public String getKey() {
		return key;
	}

	/**
	 * @param key the key to set
	 */
	@PreferenceField(fieldName="ptt_channel_key")
	public void setKey(String key) {
		this.key = key;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + ((address == null) ? 0 : address.hashCode());
		result = prime * result + ((alias == null) ? 0 : alias.hashCode());
		result = prime * result + ((key == null) ? 0 : key.hashCode());
		result = prime * result + port;
		result = prime * result + ttl;
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
		PTTChannel other = (PTTChannel) obj;
		if (address == null) {
			if (other.address != null)
				return false;
		} else if (!address.equals(other.address))
			return false;
		if (alias == null) {
			if (other.alias != null)
				return false;
		} else if (!alias.equals(other.alias))
			return false;
		if (key == null) {
			if (other.key != null)
				return false;
		} else if (!key.equals(other.key))
			return false;
		if (port != other.port)
			return false;
		if (ttl != other.ttl)
			return false;
		return true;
	}

	@Override
	public String getTitleForMenu() {
		return alias;
	}

	@Override
	public String getKeyForIntent() {
		return PTT_CHANNEL_ID;
	}

	static private void makeChannels(SQLiteDatabase db) {
		PTTChannel pttChannel;
		for(int i = 1; i <= 40; i++) {
			pttChannel = new PTTChannel();
			pttChannel.setAlias("Channel " + i);
			pttChannel.setAddress("224.0.224." + i);
			pttChannel.commit(db);
		}
	}

}
