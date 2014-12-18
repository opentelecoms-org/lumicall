package org.lumicall.android.sip;

import java.security.SecureRandom;
import java.util.List;

import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIP5060ProvisioningRequest;

import android.app.Activity;

public class RegistrationActivity extends Activity {
	
	protected final static int PASSWORD_LEN = 12;
	
	private String password;
	
    private String generatePassword(int length)	{
	    String availableCharacters = "";
	    StringBuilder password = new StringBuilder("");
	    
	    // Generate the appropriate character set
	    availableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	    availableCharacters = availableCharacters + "0123456789";
	    
	    // Generate the random number generator
	    SecureRandom selector = new SecureRandom();
	    
	    // Generate the password
	    int i;
	    for(i = 0; i < length; i++)
	    {
	            password.append(availableCharacters.charAt(selector.nextInt(availableCharacters.length())));
	    }
	    
	    return password.toString();
    }
    
    protected String getPassword() {
    	if(password == null)
    		password = generatePassword(PASSWORD_LEN);
    	return password;
    }
	
    protected void storeSettings(String phoneNumber) {
    	
    	SIP5060ProvisioningRequest req = new SIP5060ProvisioningRequest();
    	req.setPhoneNumber(phoneNumber);
    	req.setAuthPassword(getPassword());
    	
    	LumicallDataSource db = new LumicallDataSource(this);
    	db.open();
    	// clear out any previous incomplete request
    	List<SIP5060ProvisioningRequest> reqs = db.getSIP5060ProvisioningRequests();
    	for(SIP5060ProvisioningRequest _req : reqs)
    		db.deleteSIP5060ProvisioningRequest(_req);
    	// Now put in the new request we are starting
    	db.persistSIP5060ProvisioningRequest(req);
    	db.close();
    }


}
