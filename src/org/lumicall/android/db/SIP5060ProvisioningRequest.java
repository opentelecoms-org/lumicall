package org.lumicall.android.db;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.preferences.PreferenceField;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class SIP5060ProvisioningRequest extends DBObject {
	
	private final static String DB_TABLE = "SIP5060ProvisioningRequest";
	private final static String COLUMN_PHONE_NUMBER = "phone_number";
	private final static String COLUMN_VALIDATION_CODE_1 = "v_code1";
	private final static String COLUMN_VALIDATION_CODE_2 = "v_code2";
	private final static String COLUMN_CREATION_TIMESTAMP = "creation_timestamp";
	private final static String COLUMN_AUTH_PASSWORD = "auth_password";
	
	private final static String[] ALL_COLUMNS = new String[] {
		COLUMN_ID,
		COLUMN_PHONE_NUMBER,
		COLUMN_VALIDATION_CODE_1,
		COLUMN_VALIDATION_CODE_2,
		COLUMN_CREATION_TIMESTAMP,
		COLUMN_AUTH_PASSWORD
	};
	
	private final static String CREATE_TABLE =
			"CREATE TABLE " + DB_TABLE + " (" +
			COLUMN_ID + " integer primary key autoincrement, " +
			COLUMN_PHONE_NUMBER + " text, " +
			COLUMN_VALIDATION_CODE_1 + " text, " +
			COLUMN_VALIDATION_CODE_2 + " text, " +
			COLUMN_CREATION_TIMESTAMP + " integer not null, " +
			COLUMN_AUTH_PASSWORD + " text not null);";

	String phoneNumber = null;
	String validationCode1 = null;
	String validationCode2 = null;
	long creationTimestamp = 0;
	String authPassword = null;
	
	public static void onCreate(SQLiteDatabase db) {	
		db.execSQL(CREATE_TABLE);
	}
	
	public static void onUpgrade(SQLiteDatabase db, int oldVersion,
			int newVersion) {
		if(oldVersion < 6 && newVersion >= 6) {
			onCreate(db);
		}
	}
	
	public SIP5060ProvisioningRequest() {
		setCreationTimestamp(System.currentTimeMillis() / 1000);
	}
	
	public static List<SIP5060ProvisioningRequest> loadFromDatabase(SQLiteDatabase db) {
		Vector<SIP5060ProvisioningRequest> v = new Vector<SIP5060ProvisioningRequest>();
		
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
	
	public static SIP5060ProvisioningRequest loadFromDatabase(SQLiteDatabase db, long id) {
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, COLUMN_ID + " = " + id,
				null, null, null, null);
		cursor.moveToFirst();
		SIP5060ProvisioningRequest req = null;
		if(!cursor.isAfterLast())
			 req = fromCursor(cursor);
		cursor.close();
		return req;
	}
	
	private static SIP5060ProvisioningRequest fromCursor(Cursor cursor) {
		
		SIP5060ProvisioningRequest req = new SIP5060ProvisioningRequest();
		
		int i = 0;
		
		req.setId(cursor.getLong(i++));
		req.setPhoneNumber(cursor.getString(i++));
		req.setValidationCode1(cursor.getString(i++));
		req.setValidationCode1(cursor.getString(i++));
		req.setCreationTimestamp(cursor.getLong(i++));
		req.setAuthPassword(cursor.getString(i++));
		
		return req;
	}

	public final static String SIP5060_PROVISIONING_REQUEST_ID = "sip5060ProvisioningRequestId";
	
	@Override
	protected String getTableName() {
		return DB_TABLE;
	}
	
	@Override
	protected void putValues(ContentValues values) {
		values.put(COLUMN_PHONE_NUMBER, getPhoneNumber());
		values.put(COLUMN_VALIDATION_CODE_1, getValidationCode1());
		values.put(COLUMN_VALIDATION_CODE_2, getValidationCode2());
		values.put(COLUMN_CREATION_TIMESTAMP, getCreationTimestamp());
		values.put(COLUMN_AUTH_PASSWORD, getAuthPassword());
	}
	
	public String getPhoneNumber() {
		return phoneNumber;
	}
	
	public void setPhoneNumber(String phoneNumber) {
		this.phoneNumber = phoneNumber;
	}

	public String getValidationCode1() {
		return validationCode1;
	}

	public void setValidationCode1(String validationCode1) {
		this.validationCode1 = validationCode1;
	}

	public String getValidationCode2() {
		return validationCode2;
	}

	public void setValidationCode2(String validationCode2) {
		this.validationCode2 = validationCode2;
	}

	public long getCreationTimestamp() {
		return creationTimestamp;
	}

	public void setCreationTimestamp(long creationTimestamp) {
		this.creationTimestamp = creationTimestamp;
	}

	public String getAuthPassword() {
		return authPassword;
	}

	public void setAuthPassword(String authPassword) {
		this.authPassword = authPassword;
	}

	@Override
	public String getTitleForMenu() {
		return getId() + "";
	}

	@Override
	public String getKeyForIntent() {
		return SIP5060_PROVISIONING_REQUEST_ID;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result
				+ ((authPassword == null) ? 0 : authPassword.hashCode());
		result = prime * result
				+ (int) (creationTimestamp ^ (creationTimestamp >>> 32));
		result = prime * result
				+ ((phoneNumber == null) ? 0 : phoneNumber.hashCode());
		result = prime * result
				+ ((validationCode1 == null) ? 0 : validationCode1.hashCode());
		result = prime * result
				+ ((validationCode2 == null) ? 0 : validationCode2.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (!super.equals(obj))
			return false;
		if (!(obj instanceof SIP5060ProvisioningRequest))
			return false;
		SIP5060ProvisioningRequest other = (SIP5060ProvisioningRequest) obj;
		if (authPassword == null) {
			if (other.authPassword != null)
				return false;
		} else if (!authPassword.equals(other.authPassword))
			return false;
		if (creationTimestamp != other.creationTimestamp)
			return false;
		if (phoneNumber == null) {
			if (other.phoneNumber != null)
				return false;
		} else if (!phoneNumber.equals(other.phoneNumber))
			return false;
		if (validationCode1 == null) {
			if (other.validationCode1 != null)
				return false;
		} else if (!validationCode1.equals(other.validationCode1))
			return false;
		if (validationCode2 == null) {
			if (other.validationCode2 != null)
				return false;
		} else if (!validationCode2.equals(other.validationCode2))
			return false;
		return true;
	}

}
