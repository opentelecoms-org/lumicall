package org.lumicall.android.db;

import java.util.List;
import java.util.Vector;

import android.content.Context;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;

public class LumicallDataSource {

	// Database fields
	private SQLiteDatabase db;
	private DatabaseHelper helper;

	public LumicallDataSource(Context context) {
		helper = new DatabaseHelper(context);
	}

	public void open() throws SQLException {
		db = helper.getWritableDatabase();
	}

	public void close() {
		helper.close();
	}
	
	public SIPIdentity getSIPIdentity(long id) {
		return SIPIdentity.loadFromDatabase(db, id);
	}
	
	public List<SIPIdentity> getSIPIdentities() {
		return SIPIdentity.loadFromDatabase(db);
	}
	
	public void persistSIPIdentity(SIPIdentity sipIdentity) {
		sipIdentity.commit(db);
	}
	
	public void deleteSIPIdentity(SIPIdentity sipIdentity) {
		sipIdentity.delete(db);
	}

	public List<PTTChannel> getPTTChannels() {
		return PTTChannel.loadFromDatabase(db);
	}

	public void deletePTTChannel(PTTChannel object) {
		object.delete(db);
	}

	public void persistPTTChannel(PTTChannel object) {
		object.commit(db);
	}

	public PTTChannel getPTTChannel(long id) {
		return PTTChannel.loadFromDatabase(db, id);
	}
	
	public List<ENUMSuffix> getENUMSuffixes() {
		return ENUMSuffix.loadFromDatabase(db);
	}

	public void deleteENUMSuffix(ENUMSuffix object) {
		object.delete(db);
	}

	public ENUMSuffix getENUMSuffix(long id) {
		return ENUMSuffix.loadFromDatabase(db, id);
	}

	public void persistENUMSuffix(ENUMSuffix object) {
		object.commit(db);
	}
	
	public SIP5060ProvisioningRequest getSIP5060ProvisioningRequest(long id) {
		return SIP5060ProvisioningRequest.loadFromDatabase(db, id);
	}
	
	public List<SIP5060ProvisioningRequest> getSIP5060ProvisioningRequests() {
		return SIP5060ProvisioningRequest.loadFromDatabase(db);
	}
	
	public void persistSIP5060ProvisioningRequest(SIP5060ProvisioningRequest sIP5060ProvisioningRequest) {
		sIP5060ProvisioningRequest.commit(db);
	}
	
	public void deleteSIP5060ProvisioningRequest(SIP5060ProvisioningRequest sIP5060ProvisioningRequest) {
		sIP5060ProvisioningRequest.delete(db);
	}

	public UserMessage getUserMessage(long id) {
		return UserMessage.loadFromDatabase(db, id);
	}
	
	public List<UserMessage> getUserMessages() {
		return UserMessage.loadFromDatabase(db);
	}
	
	public void persistUserMessage(UserMessage userMessage) {
		userMessage.commit(db);
	}
	
	public void deleteUserMessage(UserMessage userMessage) {
		userMessage.delete(db);
	}

}
