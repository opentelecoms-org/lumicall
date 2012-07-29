package org.lumicall.android.db;

import java.util.List;
import java.util.Vector;

import org.lumicall.android.preferences.PreferenceField;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class SIPIdentity extends DBObject {
	
	private final static String DB_TABLE = "SIPIdentity";
	//private final static String COLUMN_ID = "_id";
	private final static String COLUMN_URI = "uri";
	private final static String COLUMN_ENABLE = "enable";
	private final static String COLUMN_AUTH_USER = "auth_user";
	private final static String COLUMN_AUTH_PASSWORD = "auth_password";
	private final static String COLUMN_MWI = "mwi";
	private final static String COLUMN_MMTEL = "mmtel";
	private final static String COLUMN_REG = "reg";
	private final static String COLUMN_Q = "q";
	private final static String COLUMN_REG_SERVER_NAME = "reg_server_name";
	private final static String COLUMN_REG_SERVER_PORT = "reg_server_port";
	private final static String COLUMN_REG_SERVER_PROTOCOL = "reg_server_protocol";
	private final static String COLUMN_OUTBOUND = "outbound";
	private final static String COLUMN_OUTBOUND_SERVER_NAME = "outbound_server_name";
	private final static String COLUMN_OUTBOUND_SERVER_PORT = "outbound_server_port";
	private final static String COLUMN_OUTBOUND_SERVER_PROTOCOL = "outbound_server_protocol";
	private final static String COLUMN_CARRIER_ROUTE = "carrier_route";
	private final static String COLUMN_CARRIER_INTL_PREFIX = "carrier_intl_prefix";
	private final static String COLUMN_STUN_SERVER_NAME = "stun_server_name";
	private final static String COLUMN_STUN_SERVER_PORT = "stun_server_port";
	private final static String COLUMN_STUN_SERVER_PROTOCOL = "stun_server_protocol";
	private final static String COLUMN_RINGTONE = "ringtone";
	private final static String COLUMN_SECURITY_MODE = "security_mode";
	private final static String COLUMN_STUN = "stun";
	
	private final static String[] ALL_COLUMNS = new String[] {
		COLUMN_ID,
		COLUMN_URI,
		COLUMN_ENABLE,
		COLUMN_AUTH_USER,
		COLUMN_AUTH_PASSWORD,
		COLUMN_MWI,
		COLUMN_MMTEL,
		COLUMN_REG,
		COLUMN_Q,
		COLUMN_REG_SERVER_NAME,
		COLUMN_REG_SERVER_PORT,
		COLUMN_REG_SERVER_PROTOCOL,
		COLUMN_OUTBOUND,
		COLUMN_OUTBOUND_SERVER_NAME,
		COLUMN_OUTBOUND_SERVER_PORT,
		COLUMN_OUTBOUND_SERVER_PROTOCOL,
		COLUMN_CARRIER_ROUTE,
		COLUMN_CARRIER_INTL_PREFIX,
		COLUMN_STUN_SERVER_NAME,
		COLUMN_STUN_SERVER_PORT,
		COLUMN_STUN_SERVER_PROTOCOL,
		COLUMN_RINGTONE,
		COLUMN_SECURITY_MODE,
		COLUMN_STUN
	};
	
	private final static String CREATE_TABLE =
			"CREATE TABLE " + DB_TABLE + " (" +
			COLUMN_ID + " integer primary key autoincrement, " +
			COLUMN_URI + " text not null, " +
			COLUMN_ENABLE + " int not null, " +
			COLUMN_AUTH_USER + " text, " +
			COLUMN_AUTH_PASSWORD + " text, " +
			COLUMN_MWI + " int not null, " +
			COLUMN_MMTEL + " int not null, " +
			COLUMN_REG + " int not null, " +
			COLUMN_Q + " real, " +
			COLUMN_REG_SERVER_NAME + " text, " + 
			COLUMN_REG_SERVER_PORT + " int, " +
			COLUMN_REG_SERVER_PROTOCOL + " text, " +
			COLUMN_OUTBOUND + " int not null, " +
			COLUMN_OUTBOUND_SERVER_NAME + " text, " + 
			COLUMN_OUTBOUND_SERVER_PORT + " int, " +
			COLUMN_OUTBOUND_SERVER_PROTOCOL + " text, " +
			COLUMN_CARRIER_ROUTE + " int not null, " +
			COLUMN_CARRIER_INTL_PREFIX + " text, " + 
			COLUMN_STUN_SERVER_NAME + " text, " + 
			COLUMN_STUN_SERVER_PORT + " int, " +
			COLUMN_STUN_SERVER_PROTOCOL + " text, " +
			COLUMN_RINGTONE + " text, " +
			COLUMN_SECURITY_MODE + " text, " +
			COLUMN_STUN + " int not null);";
	
	//long id = -1;
	String uri = null;
	boolean enable = true;
	String authUser = null;
	String authPassword = null;
	boolean mwi = false;
	boolean mMTel = false;
	boolean reg = true;
	float q = (float) 1.0;
	String regServerName = null;
	int regServerPort = 5061;
	String regServerProtocol = "tls";
	boolean outbound = true;
	String outboundServerName = null;
	int outboundServerPort = 5061;
	String outboundServerProtocol = "tls";
	boolean carrierRoute = true;
	String carrierIntlPrefix = null;
	boolean stun = true;
	String stunServerName = null;
	int stunServerPort = 3478;
	String stunServerProtocol = "udp";
	String ringTone = null;
	SecurityMode securityMode = SecurityMode.ZRTP; 
	
	public static void onCreate(SQLiteDatabase db) {	
		db.execSQL(CREATE_TABLE);
	}
	
	public static void onUpgrade(SQLiteDatabase db, int oldVersion,
			int newVersion) {
		if(oldVersion < 2 && newVersion >= 2) {
			db.execSQL("ALTER TABLE " + DB_TABLE +
					" ADD COLUMN " + COLUMN_SECURITY_MODE + " TEXT " +
					" DEFAULT '" + SecurityMode.ZRTP + "' NOT NULL;");
		}
		if(oldVersion < 3 && newVersion >= 3) {
			db.execSQL("ALTER TABLE " + DB_TABLE +
					" ADD COLUMN " + COLUMN_STUN + " INT " +
					" DEFAULT '1' NOT NULL;");
		}
	}
	
	public SIPIdentity() {		
	}
	
	public static List<SIPIdentity> loadFromDatabase(SQLiteDatabase db) {
		Vector<SIPIdentity> v = new Vector<SIPIdentity>();
		
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
	
	public static SIPIdentity loadFromDatabase(SQLiteDatabase db, long id) {
		
		Cursor cursor = db.query(DB_TABLE,
				ALL_COLUMNS, COLUMN_ID + " = " + id,
				null, null, null, null);
		cursor.moveToFirst();
		SIPIdentity sipIdentity = null;
		if(!cursor.isAfterLast())
			 sipIdentity = fromCursor(cursor);
		cursor.close();
		return sipIdentity;
	}
	
	private static SIPIdentity fromCursor(Cursor cursor) {
		
		SIPIdentity sipIdentity = new SIPIdentity();
		
		int i = 0;
		
		sipIdentity.setId(cursor.getLong(i++));
		sipIdentity.setUri(cursor.getString(i++));
		sipIdentity.setEnable(fromBoolean(cursor.getInt(i++)));
		sipIdentity.setAuthUser(cursor.getString(i++));
		sipIdentity.setAuthPassword(cursor.getString(i++));
		sipIdentity.setMwi(fromBoolean(cursor.getInt(i++)));
		sipIdentity.setMMTel(fromBoolean(cursor.getInt(i++)));
		sipIdentity.setReg(fromBoolean(cursor.getInt(i++)));
		sipIdentity.setQ(cursor.getFloat(i++));
		sipIdentity.setRegServerName(cursor.getString(i++));
		sipIdentity.setRegServerPort(cursor.getInt(i++));
		sipIdentity.setRegServerProtocol(cursor.getString(i++));
		sipIdentity.setOutbound(fromBoolean(cursor.getInt(i++)));
		sipIdentity.setOutboundServerName(cursor.getString(i++));
		sipIdentity.setOutboundServerPort(cursor.getInt(i++));
		sipIdentity.setOutboundServerProtocol(cursor.getString(i++));
		sipIdentity.setCarrierRoute(fromBoolean(cursor.getInt(i++)));
		sipIdentity.setCarrierIntlPrefix(cursor.getString(i++));
		sipIdentity.setStunServerName(cursor.getString(i++));
		sipIdentity.setStunServerPort(cursor.getInt(i++));
		sipIdentity.setStunServerProtocol(cursor.getString(i++));
		sipIdentity.setRingTone(cursor.getString(i++));
		sipIdentity.setSecurityMode(SecurityMode.valueOf(cursor.getString(i++)));
		sipIdentity.setStun(fromBoolean(cursor.getInt(i++)));
		
		return sipIdentity;
	}
	
	public final static String SIP_IDENTITY_ID = "sipIdentityId";
	
	@Override
	protected String getTableName() {
		return DB_TABLE;
	}
	
	@Override
	protected void putValues(ContentValues values) {
		values.put(COLUMN_URI, getUri());
		values.put(COLUMN_ENABLE, toBoolean(isEnable()));
		values.put(COLUMN_AUTH_USER, getAuthUser());
		values.put(COLUMN_AUTH_PASSWORD, getAuthPassword());
		values.put(COLUMN_MWI, toBoolean(isMwi()));
		values.put(COLUMN_MMTEL, toBoolean(isMMTel()));
		values.put(COLUMN_REG, toBoolean(isReg()));
		values.put(COLUMN_Q, getQ());
		values.put(COLUMN_REG_SERVER_NAME, getRegServerName());
		values.put(COLUMN_REG_SERVER_PORT, getRegServerPort());
		values.put(COLUMN_REG_SERVER_PROTOCOL, getRegServerProtocol());
		values.put(COLUMN_OUTBOUND, toBoolean(isOutbound()));
		values.put(COLUMN_OUTBOUND_SERVER_NAME, getOutboundServerName());
		values.put(COLUMN_OUTBOUND_SERVER_PORT, getOutboundServerPort());
		values.put(COLUMN_OUTBOUND_SERVER_PROTOCOL, getOutboundServerProtocol());
		values.put(COLUMN_CARRIER_ROUTE, toBoolean(isCarrierRoute()));
		values.put(COLUMN_CARRIER_INTL_PREFIX, getCarrierIntlPrefix());
		values.put(COLUMN_STUN_SERVER_NAME, getStunServerName());
		values.put(COLUMN_STUN_SERVER_PORT, getStunServerPort());
		values.put(COLUMN_STUN_SERVER_PROTOCOL, getStunServerProtocol());
		values.put(COLUMN_RINGTONE, getRingTone());
		values.put(COLUMN_SECURITY_MODE, securityMode.toString());
		values.put(COLUMN_STUN, toBoolean(isStun()));
	}
	
	@PreferenceField(fieldName="sip_identity_uri")
	public String getUri() {
		return uri;
	}

	@PreferenceField(fieldName="sip_identity_uri")
	public void setUri(String uri) {
		this.uri = uri;
	}
	
	@PreferenceField(fieldName="sip_identity_enable")
	public boolean isEnable() {
		return enable;
	}

	@PreferenceField(fieldName="sip_identity_enable")
	public void setEnable(boolean enable) {
		this.enable = enable;
	}

	@PreferenceField(fieldName="sip_identity_auth_user")
	public String getAuthUser() {
		return authUser;
	}

	@PreferenceField(fieldName="sip_identity_auth_user")
	public void setAuthUser(String authUser) {
		this.authUser = authUser;
	}

	@PreferenceField(fieldName="sip_identity_auth_password")
	public String getAuthPassword() {
		return authPassword;
	}

	@PreferenceField(fieldName="sip_identity_auth_password")
	public void setAuthPassword(String authPassword) {
		this.authPassword = authPassword;
	}
	
	@PreferenceField(fieldName="sip_identity_mwi")
	public boolean isMwi() {
		return mwi;
	}

	@PreferenceField(fieldName="sip_identity_mwi")
	public void setMwi(boolean mwi) {
		this.mwi = mwi;
	}
	
	@PreferenceField(fieldName="sip_identity_mmtel")
	public boolean isMMTel() {
		return mMTel;
	}

	@PreferenceField(fieldName="sip_identity_mmtel")
	public void setMMTel(boolean mMTel) {
		this.mMTel = mMTel;
	}

	@PreferenceField(fieldName="sip_identity_registration")
	public boolean isReg() {
		return reg;
	}

	@PreferenceField(fieldName="sip_identity_registration")
	public void setReg(boolean reg) {
		this.reg = reg;
	}

	@PreferenceField(fieldName="sip_identity_q")
	public float getQ() {
		return q;
	}

	@PreferenceField(fieldName="sip_identity_q")
	public void setQ(float q) {
		this.q = q;
	}

	@PreferenceField(fieldName="sip_identity_reg_server_name")
	public String getRegServerName() {
		return regServerName;
	}

	@PreferenceField(fieldName="sip_identity_reg_server_name")
	public void setRegServerName(String regServerName) {
		this.regServerName = regServerName;
	}

	@PreferenceField(fieldName="sip_identity_reg_server_port")
	public int getRegServerPort() {
		return regServerPort;
	}

	@PreferenceField(fieldName="sip_identity_reg_server_port")
	public void setRegServerPort(int regServerPort) {
		this.regServerPort = regServerPort;
	}

	@PreferenceField(fieldName="sip_identity_reg_server_protocol")
	public String getRegServerProtocol() {
		return regServerProtocol;
	}

	@PreferenceField(fieldName="sip_identity_reg_server_protocol")
	public void setRegServerProtocol(String regServerProtocol) {
		this.regServerProtocol = regServerProtocol;
	}

	@PreferenceField(fieldName="sip_identity_outbound")
	public boolean isOutbound() {
		return outbound;
	}

	@PreferenceField(fieldName="sip_identity_outbound")
	public void setOutbound(boolean outbound) {
		this.outbound = outbound;
	}

	@PreferenceField(fieldName="sip_identity_outbound_server_name")
	public String getOutboundServerName() {
		return outboundServerName;
	}

	@PreferenceField(fieldName="sip_identity_outbound_server_name")
	public void setOutboundServerName(String outboundServerName) {
		this.outboundServerName = outboundServerName;
	}

	@PreferenceField(fieldName="sip_identity_outbound_server_port")
	public int getOutboundServerPort() {
		return outboundServerPort;
	}

	@PreferenceField(fieldName="sip_identity_outbound_server_port")
	public void setOutboundServerPort(int outboundServerPort) {
		this.outboundServerPort = outboundServerPort;
	}

	@PreferenceField(fieldName="sip_identity_outbound_server_protocol")
	public String getOutboundServerProtocol() {
		return outboundServerProtocol;
	}

	@PreferenceField(fieldName="sip_identity_outbound_server_protocol")
	public void setOutboundServerProtocol(String outboundServerProtocol) {
		this.outboundServerProtocol = outboundServerProtocol;
	}
	
	@PreferenceField(fieldName="sip_identity_carrier_route")
	public boolean isCarrierRoute() {
		return carrierRoute;
	}

	@PreferenceField(fieldName="sip_identity_carrier_route")
	public void setCarrierRoute(boolean carrierRoute) {
		this.carrierRoute = carrierRoute;
	}

	@PreferenceField(fieldName="sip_identity_carrier_intl_prefix")
	public String getCarrierIntlPrefix() {
		return carrierIntlPrefix;
	}

	@PreferenceField(fieldName="sip_identity_carrier_intl_prefix")
	public void setCarrierIntlPrefix(String carrierIntlPrefix) {
		this.carrierIntlPrefix = carrierIntlPrefix;
	}

	@PreferenceField(fieldName="sip_identity_stun_server_name")
	public String getStunServerName() {
		return stunServerName;
	}

	@PreferenceField(fieldName="sip_identity_stun_server_name")
	public void setStunServerName(String stunServerName) {
		this.stunServerName = stunServerName;
	}

	@PreferenceField(fieldName="sip_identity_stun_server_port")
	public int getStunServerPort() {
		return stunServerPort;
	}

	@PreferenceField(fieldName="sip_identity_stun_server_port")
	public void setStunServerPort(int stunServerPort) {
		this.stunServerPort = stunServerPort;
	}

	@PreferenceField(fieldName="sip_identity_stun_server_protocol")
	public String getStunServerProtocol() {
		return stunServerProtocol;
	}

	@PreferenceField(fieldName="sip_identity_stun_server_protocol")
	public void setStunServerProtocol(String stunServerProtocol) {
		this.stunServerProtocol = stunServerProtocol;
	}

	@PreferenceField(fieldName="sip_identity_ringtone")
	public String getRingTone() {
		return ringTone;
	}

	@PreferenceField(fieldName="sip_identity_ringtone")
	public void setRingTone(String ringTone) {
		this.ringTone = ringTone;
	}
	
	@PreferenceField(fieldName="sip_identity_security_mode")
	public SecurityMode getSecurityMode() {
		return securityMode;
	}

	@PreferenceField(fieldName="sip_identity_security_mode")
	public void setSecurityMode(SecurityMode securityMode) {
		this.securityMode = securityMode;
	}
	
	@PreferenceField(fieldName="sip_identity_stun")
	public boolean isStun() {
		return stun;
	}

	@PreferenceField(fieldName="sip_identity_stun")
	public void setStun(boolean stun) {
		this.stun = stun;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result
				+ ((authPassword == null) ? 0 : authPassword.hashCode());
		result = prime * result
				+ ((authUser == null) ? 0 : authUser.hashCode());
		result = prime
				* result
				+ ((carrierIntlPrefix == null) ? 0 : carrierIntlPrefix
						.hashCode());
		result = prime * result + (carrierRoute ? 1231 : 1237);
		result = prime * result + (enable ? 1231 : 1237);
		result = prime * result + (mMTel ? 1231 : 1237);
		result = prime * result + (mwi ? 1231 : 1237);
		result = prime * result + (outbound ? 1231 : 1237);
		result = prime
				* result
				+ ((outboundServerName == null) ? 0 : outboundServerName
						.hashCode());
		result = prime * result + outboundServerPort;
		result = prime
				* result
				+ ((outboundServerProtocol == null) ? 0
						: outboundServerProtocol.hashCode());
		result = prime * result + Float.floatToIntBits(q);
		result = prime * result + (reg ? 1231 : 1237);
		result = prime * result
				+ ((regServerName == null) ? 0 : regServerName.hashCode());
		result = prime * result + regServerPort;
		result = prime
				* result
				+ ((regServerProtocol == null) ? 0 : regServerProtocol
						.hashCode());
		result = prime * result
				+ ((ringTone == null) ? 0 : ringTone.hashCode());
		result = prime * result
				+ ((securityMode == null) ? 0 : securityMode.hashCode());
		result = prime * result + (stun ? 1231 : 1237);
		result = prime * result
				+ ((stunServerName == null) ? 0 : stunServerName.hashCode());
		result = prime * result + stunServerPort;
		result = prime
				* result
				+ ((stunServerProtocol == null) ? 0 : stunServerProtocol
						.hashCode());
		result = prime * result + ((uri == null) ? 0 : uri.hashCode());
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
		SIPIdentity other = (SIPIdentity) obj;
		if (authPassword == null) {
			if (other.authPassword != null)
				return false;
		} else if (!authPassword.equals(other.authPassword))
			return false;
		if (authUser == null) {
			if (other.authUser != null)
				return false;
		} else if (!authUser.equals(other.authUser))
			return false;
		if (carrierIntlPrefix == null) {
			if (other.carrierIntlPrefix != null)
				return false;
		} else if (!carrierIntlPrefix.equals(other.carrierIntlPrefix))
			return false;
		if (carrierRoute != other.carrierRoute)
			return false;
		if (enable != other.enable)
			return false;
		if (mMTel != other.mMTel)
			return false;
		if (mwi != other.mwi)
			return false;
		if (outbound != other.outbound)
			return false;
		if (outboundServerName == null) {
			if (other.outboundServerName != null)
				return false;
		} else if (!outboundServerName.equals(other.outboundServerName))
			return false;
		if (outboundServerPort != other.outboundServerPort)
			return false;
		if (outboundServerProtocol == null) {
			if (other.outboundServerProtocol != null)
				return false;
		} else if (!outboundServerProtocol.equals(other.outboundServerProtocol))
			return false;
		if (Float.floatToIntBits(q) != Float.floatToIntBits(other.q))
			return false;
		if (reg != other.reg)
			return false;
		if (regServerName == null) {
			if (other.regServerName != null)
				return false;
		} else if (!regServerName.equals(other.regServerName))
			return false;
		if (regServerPort != other.regServerPort)
			return false;
		if (regServerProtocol == null) {
			if (other.regServerProtocol != null)
				return false;
		} else if (!regServerProtocol.equals(other.regServerProtocol))
			return false;
		if (ringTone == null) {
			if (other.ringTone != null)
				return false;
		} else if (!ringTone.equals(other.ringTone))
			return false;
		if (securityMode != other.securityMode)
			return false;
		if (stun != other.stun)
			return false;
		if (stunServerName == null) {
			if (other.stunServerName != null)
				return false;
		} else if (!stunServerName.equals(other.stunServerName))
			return false;
		if (stunServerPort != other.stunServerPort)
			return false;
		if (stunServerProtocol == null) {
			if (other.stunServerProtocol != null)
				return false;
		} else if (!stunServerProtocol.equals(other.stunServerProtocol))
			return false;
		if (uri == null) {
			if (other.uri != null)
				return false;
		} else if (!uri.equals(other.uri))
			return false;
		return true;
	}

	@Override
	public String getTitleForMenu() {
		return getUri();
	}

	@Override
	public String getKeyForIntent() {
		return SIP_IDENTITY_ID;
	}

}
