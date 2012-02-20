package org.lumicall.android.db;

import android.content.ContentValues;
import android.database.sqlite.SQLiteDatabase;

public abstract class DBObject {
	
	protected final static String COLUMN_ID = "_id";

	long id = -1;

	public DBObject() {	
	}
	
	protected static boolean fromBoolean(int b) {
		return b == 1;
	}
	
	protected static int toBoolean(boolean b) {
		return b ? 1 : 0;
	}

	protected abstract void putValues(ContentValues values);
	protected abstract String getTableName();
	
	public void commit(SQLiteDatabase db) {
		
		ContentValues values = new ContentValues();
		putValues(values);

		if(getId() == -1) {
			// insert and then setId()
			setId(db.insert(getTableName(), null, values));
		} else {
			// update
			values.put(COLUMN_ID, getId());
			db.replace(getTableName(), null, values);
		}
	}
	
	public void delete(SQLiteDatabase db) {
		if(getId() != -1) {
			db.execSQL("DELETE FROM " + getTableName() +
				" WHERE " + COLUMN_ID + " = " + getId() +
				";");
		}
		setId(-1);		
	}

	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public abstract String getTitleForMenu();
	
	public abstract String getKeyForIntent();

}
