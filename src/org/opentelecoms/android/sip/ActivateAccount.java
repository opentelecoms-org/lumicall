package org.opentelecoms.android.sip;

import org.opentelecoms.android.reg.RegistrationPhaseTwo;
import org.sipdroid.sipua.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class ActivateAccount extends Activity {
	
	private final static int REG_CODE_SIZE = 10;
	
	private EditText etCode;
	private Button buttonOK;
	
	protected void activateAccountNow() {
		String regCode = etCode.getText().toString();
		if(validateRegCode(regCode)) {
			Toast.makeText(this, R.string.reg_please_wait, Toast.LENGTH_LONG).show();
			handleRegistrationCode(regCode);
		} else {
			Toast.makeText(this, R.string.activate_invalid_code, Toast.LENGTH_LONG).show();
		}
	}
		
	protected boolean validateRegCode(String regCode) {
		
		if(regCode.length() != REG_CODE_SIZE)
			return false;
		
		return true;
	}

	protected void handleRegistrationCode(String regCode) {
		final Intent intent = new Intent(RegistrationPhaseTwo.ACTION);
		//intent.setAction(RegistrationPhaseTwo.ACTION);
		intent.putExtra(RegistrationPhaseTwo.REG_CODE, regCode);
		sendBroadcast(intent);
	}

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activate_dialog);
        setTitle("Complete service activation");
        buttonOK = (Button) findViewById(R.id.Button01);
		buttonOK.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				activateAccountNow();
			}
		});
        etCode = (EditText) findViewById(R.id.EditText01);
        
        // TODO: we should check for any SMS in the inbox,
        // and activate automatically if one is found
        
    }

}
