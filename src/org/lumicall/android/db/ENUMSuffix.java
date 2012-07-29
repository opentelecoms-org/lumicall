package org.lumicall.android.db;

import java.util.Arrays;
import java.util.List;
import java.util.Vector;

import org.lumicall.android.AppProperties;
import org.lumicall.android.preferences.PreferenceField;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class ENUMSuffix extends DBObject implements Comparable<ENUMSuffix> {
	
	private final static String[] DEFAULT_SUFFIXES =
			new String[] {
				"e164.arpa",
				"e164-addr.sip5060.net",
				"e164.org"
	};
	
	private final static String DB_TABLE = "ENUMSuffix";
	private final static String COLUMN_ALIAS = "alias";
	private final static String COLUMN_SUFFIX = "suffix";
	private final static String COLUMN_PRIORITY = "priority";
	
	private final static String[] ALL_COLUMNS = new String[] {
		COLUMN_ID,
		COLUMN_ALIAS,
		COLUMN_SUFFIX,
		COLUMN_PRIORITY
	};
	
	private final static String CREATE_TABLE =
			"CREATE TABLE " + DB_TABLE + " (" +
			COLUMN_ID + " integer primary key autoincrement, " +
			COLUMN_ALIAS + " text not null, " +
			COLUMN_SUFFIX + " text not null, " +
			COLUMN_PRIORITY + " int not null);";
	
	String alias = null;
	String suffix = null;
	// suffix with highest priority value takes precedence
	int priority = 0;
	
	public static void onCreate(SQLiteDatabase db) {	
		db.execSQL(CREATE_TABLE);
		addDefaultSuffixes(db);
	}
	
	private static void addDefaultSuffixes(SQLiteDatabase db) {
		int p = DEFAULT_SUFFIXES.length - 1; 
		for(String s : DEFAULT_SUFFIXES) {
			ENUMSuffix e = new ENUMSuffix();
			e.setAlias(s);
			e.setSuffix(s);
			e.setPriority(p--);
			e.commit(db);
		}
	}

	public static void onUpgrade(SQLiteDatabase db, int oldVersion,
			int newVersion) {
		if(oldVersion < 5 && newVersion >= 5) {
			// This table didn't exist in DB versions < 5
			onCreate(db);
		}
	}
	
	public ENUMSuffix() {		
	}
	
	public static List<ENUMSuffix> loadFromDatabase(SQLiteDatabase db) {
		Vector<ENUMSuffix> v = new Vector<ENUMSuffix>();
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, null,
				null, null, null, null);
		cursor.moveToFirst();
		while(!cursor.isAfterLast()) {
			v.add(fromCursor(cursor));
			cursor.moveToNext();
		}
		cursor.close();
		ENUMSuffix[] _r = v.toArray(new ENUMSuffix[] {});
		v.clear();
		Arrays.sort(_r);
		for(ENUMSuffix e : _r)
			v.add(e);
		return v;
	}
	
	public static ENUMSuffix loadFromDatabase(SQLiteDatabase db, long id) {
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, COLUMN_ID + " = " + id,
				null, null, null, null);
		cursor.moveToFirst();
		ENUMSuffix enumSuffix = null;
		if(!cursor.isAfterLast())
			 enumSuffix = fromCursor(cursor);
		cursor.close();
		return enumSuffix;
	}
	
	private static ENUMSuffix fromCursor(Cursor cursor) {
		
		ENUMSuffix enumSuffix = new ENUMSuffix();
		
		int i = 0;
		
		enumSuffix.setId(cursor.getLong(i++));
		enumSuffix.setAlias(cursor.getString(i++));
		enumSuffix.setSuffix(cursor.getString(i++));
		enumSuffix.setPriority(cursor.getInt(i++));
		
		return enumSuffix;
	}
	
	public final static String ENUM_SUFFIX_ID = "enumSuffixId";
	
	@Override
	protected String getTableName() {
		return DB_TABLE;
	}
	
	@Override
	protected void putValues(ContentValues values) {
		values.put(COLUMN_ALIAS, getAlias());
		values.put(COLUMN_SUFFIX, getSuffix());
		values.put(COLUMN_PRIORITY, getPriority());
	}
	
	@PreferenceField(fieldName="enum_suffix_alias")
	public String getAlias() {
		return alias;
	}

	@PreferenceField(fieldName="enum_suffix_alias")
	public void setAlias(String alias) {
		this.alias = alias;
	}

	@PreferenceField(fieldName="enum_suffix_suffix")
	public String getSuffix() {
		return suffix;
	}

	@PreferenceField(fieldName="enum_suffix_suffix")
	public void setSuffix(String suffix) {
		this.suffix = suffix;
	}

	@PreferenceField(fieldName="enum_suffix_priority")
	public int getPriority() {
		return priority;
	}

	@PreferenceField(fieldName="enum_suffix_priority")
	public void setPriority(int priority) {
		this.priority = priority;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + ((alias == null) ? 0 : alias.hashCode());
		result = prime * result + priority;
		result = prime * result + ((suffix == null) ? 0 : suffix.hashCode());
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
		ENUMSuffix other = (ENUMSuffix) obj;
		if (alias == null) {
			if (other.alias != null)
				return false;
		} else if (!alias.equals(other.alias))
			return false;
		if (priority != other.priority)
			return false;
		if (suffix == null) {
			if (other.suffix != null)
				return false;
		} else if (!suffix.equals(other.suffix))
			return false;
		return true;
	}

	@Override
	public String getTitleForMenu() {
		return getAlias();
	}

	@Override
	public String getKeyForIntent() {
		return ENUM_SUFFIX_ID;
	}

	@Override
	public int compareTo(ENUMSuffix arg0) {
		return arg0.getPriority() - getPriority(); 
	}

}
