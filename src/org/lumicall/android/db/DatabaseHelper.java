package org.lumicall.android.db;

import java.util.logging.Logger;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class DatabaseHelper extends SQLiteOpenHelper {
	
	private Logger logger = Logger.getLogger(getClass().getName());
	
	private final static String DB_NAME = "lumicall";
	private final static int DB_VERSION = 7;
	
	public DatabaseHelper(Context context) {
		super(context, DB_NAME, null, DB_VERSION);
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
		SIPIdentity.onCreate(db);
		PTTChannel.onCreate(db);
		ENUMSuffix.onCreate(db);
		SIP5060ProvisioningRequest.onCreate(db);
		UserMessage.onCreate(db);
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		logger.info("Upgrading DB from version " + oldVersion + " to version " + newVersion);
		SIPIdentity.onUpgrade(db, oldVersion, newVersion);
		PTTChannel.onUpgrade(db, oldVersion, newVersion);
		ENUMSuffix.onUpgrade(db, oldVersion, newVersion);
		SIP5060ProvisioningRequest.onUpgrade(db, oldVersion, newVersion);
		UserMessage.onUpgrade(db, oldVersion, newVersion);
	}
}
