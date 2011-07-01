package org.opentelecoms.android.sip;

import java.util.Date;

import org.opentelecoms.android.reg.EnrolmentService;
import org.sipdroid.sipua.R;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class ActivateAccount extends Activity {
	
	private final static int REG_CODE_SIZE = 10;
	private final static String DEFAULT_REG_CODE = "**";
	
	private EditText etCode;
	private Button buttonOK;

	private Button buttonRegAgain;
	
	private static final String TAG = "ActAcct";
	
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
		
		if(regCode.length() != REG_CODE_SIZE)
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
				SharedPreferences settings = getSharedPreferences(RegisterAccount.PREFS_FILE, MODE_PRIVATE);
		    	Editor ed = settings.edit();
		    	ed.putLong(RegisterAccount.PREF_LAST_ENROLMENT_ATTEMPT, 0);
		    	ed.putLong(RegisterAccount.PREF_LAST_VALIDATION_ATTEMPT, 0);
		    	ed.commit();
				final Intent intent = new Intent(ActivateAccount.this, RegisterAccount.class);
		        Log.v(TAG, "user wants to try registration form again");
		        startActivity(intent);
		        finish();
			}
		});
		
        etCode = (EditText) findViewById(R.id.EditText01);
        etCode.setText(DEFAULT_REG_CODE);
        
        // TODO: we should check for any SMS in the inbox,
        // and activate automatically if one is found
        
    }

}
