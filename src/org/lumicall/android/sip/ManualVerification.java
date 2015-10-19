package org.lumicall.android.sip;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

import org.lumicall.android.R;
import org.lumicall.android.Util;
import org.lumicall.android.reg.EnrolmentService;
import org.opentelecoms.util.csv.CSVReader;

import com.google.i18n.phonenumbers.NumberParseException;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.google.i18n.phonenumbers.Phonenumber.PhoneNumber;

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

public class ManualVerification extends RegistrationActivity {
	
	private static final String TAG = "ManualVerification";
	private Button buttonVerify;
	private Spinner countrySpinner;
	private EditText numberField;
	private ArrayAdapter<CountryData> countries;
	private Map<String,Integer> countriesByPosition;
	private String line1Number;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.manual_verification);
        setTitle(R.string.reg_title);
        
        countrySpinner = (Spinner) findViewById(R.id.NumberCountrySpinner);
        if(countrySpinner == null) {
        	Log.e(TAG, "countrySpinner == null");
        }
        countries = new ArrayAdapter<CountryData>(this, android.R.layout.simple_spinner_item);
        loadCountryCodes();
        countrySpinner.setAdapter(countries);
        String countryIso2 = Util.detectCountry(this);
        if(countryIso2 != null) {
        	countryIso2 = countryIso2.toLowerCase();
        	Log.i(TAG, "Setting spinner to country: " + countryIso2);
        	Integer position = countriesByPosition.get(countryIso2);
        	if(position != null) {
        		countrySpinner.setSelection(position);
        	} else {
        		// TODO - should we disable countrySpinner and insist that
        		//        the user enters their number in E.164 format?
        		//      - can we keep the list up-to-date using
        		//        libphonenumber, do they provide an ISO2 list?
        		Log.w(TAG, "unknown country/not in CSV country code list: " + countryIso2);
        	}
        }
        
        detectLine1Number();
        numberField = (EditText) findViewById(R.id.user_number);
        if(line1Number != null) {
        	numberField.setText(line1Number);
        }
        
        buttonVerify = (Button) findViewById(R.id.ButtonVerifyManual);
		buttonVerify.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				
				CountryData cd = (CountryData)countrySpinner.getSelectedItem();
				String countryIso2 = cd.getIsoCountryCode();
				Log.i(TAG, "Selected country (ISO2): " + countryIso2);
				String userNumber = numberField.getText().toString();

				boolean validNumber = false;
				PhoneNumber pn = null;
				PhoneNumberUtil pnu = PhoneNumberUtil.getInstance();
				try {
					pn = pnu.parse(userNumber, countryIso2.toUpperCase());
					if(pnu.isValidNumber(pn)) {
						Log.i(TAG, "Number appears to be valid");
						validNumber = true;
					} else {
						Log.i(TAG, "Number does not appear to be valid");
					}
				} catch (NumberParseException e) {
					Log.i(TAG, "Exception while parsing user number: " + e.getMessage());
				}
				
				if(!validNumber) {
					//Log.i(TAG, "Number not valid, asking user to correct it");
					//Toast.makeText(ManualVerification.this, R.string.reg_num_invalid_message_brief, Toast.LENGTH_LONG).show();
					//return;
					Log.w(TAG, "Number does not appear valid, using anyway");
				}
				
				String phoneNumberE164 = pnu.format(pn, PhoneNumberFormat.E164); 
				doValidation(phoneNumberE164);
			}
		});
		
        
    }
	
	@Override
	public void onDestroy() {
		/*if(mReceiver != null) {
			unregisterReceiver(mReceiver);
			mReceiver = null;
		}*/
		super.onDestroy();
	}
	
	// TODO: all ISO country codes
	private void loadCountryCodes() {
		
		countriesByPosition = new HashMap<String,Integer>();
		
		try {
			CSVReader csv = new CSVReader(CountryData.class,
					new Class[] {String.class, String.class, String.class});
			
			Resources res = getResources();
			InputStream i = res.openRawResource(R.raw.country_codes);
			BufferedReader in = new BufferedReader(new InputStreamReader(i));
			CountryData cd = (CountryData)csv.read(in);
			int position = 0;
			while(cd != null) {
				countries.add(cd);
				countriesByPosition.put(cd.getIsoCountryCode().toLowerCase(), position++);
				cd = (CountryData)csv.read(in);
			}
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			Log.e(TAG, "failed to load countries: " + e.getMessage());
			e.printStackTrace();
		}
	}

	private void detectLine1Number() {
		try {
			TelephonyManager mTelephonyMgr;
			mTelephonyMgr = (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);

		
			String line1Number = mTelephonyMgr.getLine1Number();
			if(line1Number.length() == 0) {
				line1Number = null;
			}
		} catch (Exception ex) {
			// we just ignore and try the next one...
			Log.d(TAG, "failed to get line1Number: " + ex.getMessage());
		}
	}
	
	private void doValidation(String phoneNumber) {
		buttonVerify.setEnabled(false);
		storeSettings(phoneNumber);
		
		final Intent intent = new Intent(this, EnrolmentService.class);
		intent.setAction(EnrolmentService.ACTION_ENROL_SMS);
		//intent.setAction(RegistrationPhaseTwo.ACTION);
		startService(intent);
		Log.v(TAG, "validation intent sent");
		
		finish();
	}
}
