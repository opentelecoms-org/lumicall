package org.opentelecoms.android;

import java.lang.reflect.Method;

import android.content.Context;
import android.os.RemoteException;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;

/*
 * android.permission.READ_PHONE_STATE
   android.permission.MODIFY_PHONE_STATE
   android.permission.CALL_PHONE
 */
public class PhoneHelper {

	private static String TAG = "PhoneHelper";

	/**
	 * TelephonyManager instance used by this activity
	 */
	private TelephonyManager tm;

	/**
	 * AIDL access to the telephony service process
	 */
	private ITelephony telephonyService;

	private Phone phone;

	public void init(Context context) {
		// grab an instance of telephony manager
		tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

		// connect to the underlying Android telephony system
		connectToTelephonyService(context);
	}

	/** From Tedd's source 
	 * http://code.google.com/p/teddsdroidtools/source/browse/
	 * get an instance of ITelephony to talk handle calls with 
	 */
	@SuppressWarnings("unchecked") private void connectToTelephonyService(Context context) {
		try 
		{
			// "cheat" with Java reflection to gain access to TelephonyManager's ITelephony getter
			Class c = Class.forName(tm.getClass().getName());
			Method m = c.getDeclaredMethod("getITelephony");
			m.setAccessible(true);
			telephonyService = (ITelephony)m.invoke(tm);

			makeDefaultPhone(context);
			phone = (Phone)getDefaultPhone(context);

		} catch (Exception e) {
			e.printStackTrace();
			Log.e("call prompt","FATAL ERROR: could not connect to telephony subsystem");
			Log.e("call prompt","Exception object: "+e);
		}               
	}

	/**
	 * AIDL/ITelephony technique for answering the phone
	 */
	private void answerCallAidl() {
		try {
			telephonyService.silenceRinger();
			telephonyService.answerRingingCall();
		} catch (RemoteException e) {
			e.printStackTrace();
			Log.e("call prompt","FATAL ERROR: call to service method answerRiningCall failed.");
			Log.e("call prompt","Exception object: "+e);
		}               
	}

	/** 
	 * AIDL/ITelephony technique for ignoring calls
	 */
	private void ignoreCallAidl() {
		try 
		{
			telephonyService.silenceRinger();
			telephonyService.endCall();
		} 
		catch (RemoteException e) 
		{
			e.printStackTrace();
			Log.e("call prompt","FATAL ERROR: call to service method endCall failed.");
			Log.e("call prompt","Exception object: "+e);
		}
	}

	public void answer() {
		//success = true;

		//special thanks the auto answer open source app
		//which demonstrated this answering functionality
		//Intent answer = new Intent(Intent.ACTION_MEDIA_BUTTON);
		//answer.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_HEADSETHOOK));
		//sendOrderedBroadcast(answer, null);

		//due to inconsistency, replaced with more reliable cheat method Tedd discovered
		answerCallAidl();

		//moveTaskToBack(true);
		//finish();
	}

	void reject() {
		//success = true;

		ignoreCallAidl();

		//moveTaskToBack(true);
		//finish();
	}

	public String getLine1Number() {
		try {
			return phone.getLine1Number();
		} catch (RemoteException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.e("call prompt","FATAL ERROR: call to service method getLine1Number failed.");
			Log.e("call prompt","Exception object: "+e);
		}
		return null;
	}


	public static Object getDefaultPhone(Context context) throws IllegalArgumentException {

		Object ret= null;

		try{

			ClassLoader cl = context.getClassLoader(); 
			@SuppressWarnings("rawtypes")
			Class cPhoneFactory = cl.loadClass("com.android.internal.telephony.PhoneFactory");

			Method get = cPhoneFactory.getMethod("getDefaultPhone",  (Class[]) null);
			ret= (Object)get.invoke(null, (Object[]) null);

		}catch( IllegalArgumentException iAE ){
			throw iAE;
		}catch( Exception e ){
			Log.e(TAG , "getDefaultPhone", e);
		}

		return ret;

	}

	public static Phone getCdmaPhone(Context context) throws IllegalArgumentException {

		Phone ret= null;

		try{

			ClassLoader cl = context.getClassLoader(); 
			@SuppressWarnings("rawtypes")
			Class PhoneFactory = cl.loadClass("com.android.internal.telephony.PhoneFactory");

			Method get = PhoneFactory.getMethod("getCdmaPhone",  (Class[]) null);
			ret= (Phone)get.invoke(null, (Object[]) null);

		}catch( IllegalArgumentException iAE ){
			throw iAE;
		}catch( Exception e ){
			//
		}

		return ret;

	}

	public static Phone getGsmPhone(Context context) throws IllegalArgumentException {

		Phone ret= null;

		try{

			ClassLoader cl = context.getClassLoader(); 
			@SuppressWarnings("rawtypes")
			Class PhoneFactory = cl.loadClass("com.android.internal.telephony.PhoneFactory");

			Method get = PhoneFactory.getMethod("getGsmPhone",  (Class[]) null);
			ret= (Phone)get.invoke(null, (Object[]) null);

		}catch( IllegalArgumentException iAE ){
			throw iAE;
		}catch( Exception e ){
			//
		}

		return ret;

	}

	public static void makeDefaultPhone(Context context) throws IllegalArgumentException {

		try{

			ClassLoader cl = context.getClassLoader(); 
			@SuppressWarnings("rawtypes")
			Class cPhoneFactory = cl.loadClass("com.android.internal.telephony.PhoneFactory");

			//Parameters Types
			@SuppressWarnings("rawtypes")
			Class[] paramTypes= new Class[1];
			paramTypes[0]= Context.class;

			Method get = cPhoneFactory.getMethod("makeDefaultPhone",  paramTypes);

			//Parameters
			Object[] params= new Object[1];
			params[0]= context;

			get.invoke(null, params);

		}catch( IllegalArgumentException iAE ){
			throw iAE;
		}catch( Exception e ){
			Log.e(TAG, "makeDefaultPhone", e);
		}

	}


}
