package org.lumicall.android.sip;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Vector;

import org.lumicall.android.R;
import org.lumicall.android.reg.EnrolmentService;
import org.opentelecoms.util.csv.CSVReader;

import com.google.i18n.phonenumbers.NumberParseException;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.google.i18n.phonenumbers.Phonenumber.PhoneNumber;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
	private Map<String,String> countriesByMCC;
	private Map<String,Integer> countriesByPosition;
	private String line1Number;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.manual_verification);
        setTitle(R.string.reg_title);
        
        countrySpinner = (Spinner) findViewById(R.id.NumberCountrySpinner);
        countries = new ArrayAdapter<CountryData>(this, android.R.layout.simple_spinner_item);
        loadCountryCodes();
        countrySpinner.setAdapter(countries);
        String countryIso2 = detectCountry();
        if(countryIso2 != null) {
        	countryIso2 = countryIso2.toLowerCase();
        	countrySpinner.setSelection(countriesByPosition.get(countryIso2));
        }
        
        numberField = (EditText) findViewById(R.id.user_number);
        if(line1Number != null) {
        	numberField.setText(line1Number);
        }
        
        buttonVerify = (Button) findViewById(R.id.ButtonVerifyManual);
		buttonVerify.setOnClickListener(new Button.OnClickListener() {
			public void onClick(View v) {
				
				boolean validationStarted = true;
				
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
					Log.i(TAG, "Number not valid, asking user to correct it");
					Toast.makeText(ManualVerification.this, R.string.reg_num_invalid_message_brief, Toast.LENGTH_LONG).show();
					return;
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
		
		countriesByMCC = new HashMap<String,String>();
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
			
			i = res.openRawResource(R.raw.mcc);
			in = new BufferedReader(new InputStreamReader(i));
			String line = in.readLine();
			while(line != null) {
				if(line.length() > 3) {
					String iso = line.substring(0, 1);
					String mcc = line.substring(3);
					countriesByMCC.put(mcc, iso.toLowerCase());
				}
				line = in.readLine();
			}
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			Log.e(TAG, "failed to load countries: " + e.getMessage());
			e.printStackTrace();
		}
	}

	private String detectCountry() {
		
		try {
			TelephonyManager mTelephonyMgr;
			mTelephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
			
			try {
				line1Number = mTelephonyMgr.getLine1Number();
				if(line1Number.length() == 0) {
					line1Number = null;
				}
			} catch (Exception ex) {
				// we just ignore and try the next one...
				Log.d(TAG, "failed to get line1Number: " + ex.getMessage());
			}
			
			String phoneType = null;
			try {
				int phoneTypeId = mTelephonyMgr.getPhoneType();
				switch(phoneTypeId) {
				case TelephonyManager.PHONE_TYPE_NONE:
					phoneType = "NONE";  // we can't actually do SMS enrolment in this case
					break;
				case TelephonyManager.PHONE_TYPE_GSM:
					phoneType = "GSM";
					break;
				case TelephonyManager.PHONE_TYPE_CDMA:
					phoneType = "CDMA";
					break;
				default:
					phoneType = "UNKNOWN:" + phoneTypeId;
				}
			} catch (Exception ex) {
				// we just ignore and try the next one...
				Log.d(TAG, "failed to get phoneType: " + ex.getMessage());
			}
			if (phoneType != null && phoneType.equals("CDMA")) {
				Class<?> c = null;
				Method get = null;
				try {
					// For CDMA phones
					c = Class.forName("android.os.SystemProperties");
					get = c.getMethod("get", String.class);
					// Gives MCC + MNC
					String homeOperator = ((String) get.invoke(c, "ro.cdma.home.operator.numeric"));
					String mcc = homeOperator.substring(0, 3);
					String iso2 = countriesByMCC.get(mcc);
					if(iso2 != null) {
						return iso2;
					}
				} catch (Exception ex) {
					// we just ignore and try the next one...
					Log.d(TAG, "failed to get ro.cdma.home.operator.numeric: " + ex.getMessage());
				}
			}
			try {
				String simCountry = mTelephonyMgr.getSimCountryIso();
				if(simCountry != null) {
					return simCountry;
				}
			} catch (Exception ex) {
				// we just ignore and try the next one...
				Log.d(TAG, "failed to get simCountry: " + ex.getMessage());
			}
			try {
				String networkCountry = mTelephonyMgr.getNetworkCountryIso();
				if(networkCountry != null) {
					return networkCountry;
				}
			} catch (Exception ex) {
				// we just ignore and try the next one...
				Log.d(TAG, "failed to get networkCountry: " + ex.getMessage());
			}
		} catch (Exception e) {
			Log.d(TAG, "failed to get data from TelephonyManager: " + e.getMessage());
		}
		
		String locale = Locale.getDefault().toString();
		
		if(locale != null) {
			int i = locale.indexOf('_');
			if(i >= 0) {
				return locale.substring(i, i+2);
			}
			return locale.substring(0, 2);
		}
		
		return null;
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
