package org.lumicall.android.sip;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.opentelecoms.util.Base64;
import org.lumicall.android.AppProperties;
import org.lumicall.android.R;
import org.lumicall.android.db.SIP5060ProvisioningRequest;
import org.xmlpull.v1.XmlSerializer;

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;
import android.util.Xml;

public class RegistrationUtil {

	public final static String NS = "http://opentelecoms.org/sipdroid/reg";
	
	public final static String PHASE1_PATTERN = "^(\\w+):(.\\d+)$";
	
	Logger logger = Logger.getLogger(getClass().getCanonicalName());
	
	public static void serializeProperty(XmlSerializer serializer, String ns, String propertyName, String value) throws IllegalArgumentException, IllegalStateException, IOException {
		serializer.startTag(ns, propertyName);
		serializer.text(value);
		serializer.endTag(ns, propertyName);
	}
	
	public static void serializeProperty(XmlSerializer serializer, String ns, String propertyName, int value) throws IllegalArgumentException, IllegalStateException, IOException {
		serializer.startTag(ns, propertyName);
		serializer.text(Integer.toString(value));
		serializer.endTag(ns, propertyName);
	}
	
	public static String getPublicKey(Context context) throws IOException {
		Resources res = context.getResources();
		InputStream i = res.openRawResource(R.raw.reg_server);
		BufferedReader br = new BufferedReader(new InputStreamReader(i));
		StringBuilder sb = new StringBuilder();
		String line = null;
		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}
		br.close();
		return sb.toString();
	}
	
	public static String getEncryptedStringAsBase64(Context context, String s) throws NoSuchAlgorithmException, IOException, InvalidKeySpecException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException, NoSuchProviderException {

		
		//Security.addProvider(new org.bouncycastle.jce.provider.BouncyCastleProvider());

		KeyFactory keyFactory = KeyFactory.getInstance("RSA");
		X509EncodedKeySpec pubSpec = new X509EncodedKeySpec(Base64.decode(RegistrationUtil.getPublicKey(context)));
		Key encryptionKey = keyFactory.generatePublic(pubSpec);

		//PKCS8EncodedKeySpec privSpec = new PKCS8EncodedKeySpec(Base64.decodeBase64(PVK));
		//Key encryptionKey = keyFactory.generatePrivate(privSpec);

		Cipher rsa;
		rsa = Cipher.getInstance("RSA");
		rsa.init(Cipher.ENCRYPT_MODE, encryptionKey);
		byte[] encBody = rsa.doFinal(s.getBytes());

		return Base64.encodeBytes(encBody);

	}
	
	// Fast Implementation
	private static String inputStreamToString(InputStream is) throws IOException {
	    String line = "";
	    StringBuilder total = new StringBuilder();
	    
	    // Wrap a BufferedReader around the InputStream
	    BufferedReader rd = new BufferedReader(new InputStreamReader(is));

	    // Read response until the end
	    while ((line = rd.readLine()) != null) { 
	        total.append(line); 
	    }
	    
	    // Return full string
	    return total.toString();
	}

	
	public static String submitMessage(AppProperties props, String route, String message,
			SIP5060ProvisioningRequest req) throws RegistrationFailedException {
		Logger _logger = Logger.getLogger(RegistrationUtil.class.getCanonicalName());
		
		HttpClient httpclient = new DefaultHttpClient();  
		HttpPost httppost = new HttpPost(props.getRegistrationUrl() + "/" + route);  

		String responseText = null;
		
		try {  
			  
			
			StringEntity en = new StringEntity(message);
			en.setContentType("text/xml");
			httppost.setEntity(en);

			// Execute HTTP Post Request  
			HttpResponse response = httpclient.execute(httppost);
			responseText = inputStreamToString(response.getEntity().getContent());
			
			boolean parseResponse = req != null &&
				(req.getPhoneNumber() == null || req.getPhoneNumber().length() == 0);
			if(parseResponse) {
				Pattern p = Pattern.compile(PHASE1_PATTERN);
				Matcher m = p.matcher(responseText);
				if(!m.matches()) {
					_logger.info("failed to match the response " + responseText);
					return "ERROR";
				}
				String validationCode1 = m.group(1);
				String numberToCall = m.group(2);
				req.setValidationCode1(validationCode1);
				_logger.info("validation instruction, code1 = " + validationCode1 + ", dest = " + numberToCall);
				responseText = numberToCall;
			}

		} catch (ClientProtocolException e) {  
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RegistrationFailedException (e.toString());
		} catch (IOException e) {  
			// TODO Auto-generated catch block
			throw new RegistrationFailedException (e.toString());
		}
		return responseText;
	}
	
}
