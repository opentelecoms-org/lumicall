package org.lumicall.android.sip;

import java.util.Date;

import org.lumicall.android.R;
import org.lumicall.android.db.LumicallDataSource;
import org.lumicall.android.db.SIP5060ProvisioningRequest;
import org.lumicall.android.reg.EnrolmentService;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class ActivateAccount extends Activity {
	
	private final static int REG_CODE_SIZE = 12;
	private final static String DEFAULT_REG_CODE = "";
	
	private EditText etCode;
	private Button buttonOK;

	private Button buttonRegAgain;
	private Button buttonManual;
	
	private static final String TAG = "ActAcct";
	
	BroadcastReceiver mReceiver = null;
	
	public class IncomingReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(EnrolmentService.VALIDATION_ATTEMPTED)) {
				Log.v(TAG, "notified that validation attempted, dismissing activity for account activation");
				ActivateAccount.this.finish();
			}
		}
	}
	
	protected boolean activateAccountNow() {
		String regCode = etCode.getText().toString();
		if(validateRegCode(regCode)) {
			Toast.makeText(this, R.string.reg_please_wait, Toast.LENGTH_LONG).show();
			return handleRegistrationCode(regCode);
		} else {
			Toast.makeText(this, R.string.activate_invalid_code, Toast.LENGTH_LONG).show();
			return false;
		}
	}
		
	protected boolean validateRegCode(String regCode) {
		
		if(regCode.length() < REG_CODE_SIZE)
			return false;
		
		return true;
	}

	protected boolean handleRegistrationCode(String regCode) {
		final Intent intent = new Intent(this, EnrolmentService.class);
		intent.setAction(EnrolmentService.ACTION_VALIDATE);
		//intent.setAction(RegistrationPhaseTwo.ACTION);
		intent.putExtra(EnrolmentService.VALIDATION_CODE, regCode);
		startService(intent);
		return true;
	}

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        IntentFilter filter = new IntentFilter();
        filter.addAction(EnrolmentService.VALIDATION_ATTEMPTED);
        mReceiver = new IncomingReceiver();
        registerReceiver(mReceiver, filter);
        
        setContentView(R.layout.activate_dialog);
        setTitle("Complete service activation");
        buttonOK = (Button) findViewById(R.id.Button01);
		buttonOK.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				if(activateAccountNow()) {
					// Now we give user the main screen
					//final Intent intent = new Intent(ActivateAccount.this,
					//		org.sipdroid.sipua.ui.Sipdroid.class);
					Log.v(TAG, "validation intent sent");
					//startActivity(intent);
					finish();
				}
			}
		});
		
		buttonRegAgain = (Button) findViewById(R.id.Button02);
		buttonRegAgain.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				LumicallDataSource ds = new LumicallDataSource(ActivateAccount.this);
				ds.open();
				for(SIP5060ProvisioningRequest req : ds.getSIP5060ProvisioningRequests())
					ds.deleteSIP5060ProvisioningRequest(req);
				ds.close();
				SharedPreferences settings = getSharedPreferences(RegisterAccount.PREFS_FILE, MODE_PRIVATE);
		    	Editor ed = settings.edit();
		    	ed.putBoolean(RegisterAccount.PREF_ADVANCED_SETUP, false);
		    	ed.commit();
				final Intent intent = new Intent(ActivateAccount.this, RegisterAccount.class);
		        Log.v(TAG, "user wants to try registration form again");
		        startActivity(intent);
		        finish();
			}
		});
		
		buttonManual = (Button) findViewById(R.id.ButtonActivateTryManual);
		buttonManual.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				LumicallDataSource ds = new LumicallDataSource(ActivateAccount.this);
				ds.open();
				for(SIP5060ProvisioningRequest req : ds.getSIP5060ProvisioningRequests())
					ds.deleteSIP5060ProvisioningRequest(req);
				ds.close();
				SharedPreferences settings = getSharedPreferences(RegisterAccount.PREFS_FILE, MODE_PRIVATE);
		    	Editor ed = settings.edit();
		    	ed.putBoolean(RegisterAccount.PREF_ADVANCED_SETUP, false);
		    	ed.commit();
				final Intent intent = new Intent(ActivateAccount.this, ManualVerification.class);
		        Log.v(TAG, "user wants to try manual registration");
		        startActivity(intent);
		        finish();
			}
		});
		
        etCode = (EditText) findViewById(R.id.EditText01);
        etCode.setText(DEFAULT_REG_CODE);
        
        // TODO: we should check for any SMS in the inbox,
        // and activate automatically if one is found
        
    }
	
	@Override
	public void onDestroy() {
		if(mReceiver != null) {
			unregisterReceiver(mReceiver);
			mReceiver = null;
		}
		super.onDestroy();
	}

}
