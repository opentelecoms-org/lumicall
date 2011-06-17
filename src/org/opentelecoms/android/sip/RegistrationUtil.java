package org.opentelecoms.android.sip;

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
import org.sipdroid.sipua.R;
import org.xmlpull.v1.XmlSerializer;

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;
import android.util.Xml;

public class RegistrationUtil {

	protected final static String REG_URL = "http://192.168.1.3:8086/test2";
	
	public static void serializeProperty(XmlSerializer serializer, String propertyName, String value) throws IllegalArgumentException, IllegalStateException, IOException {
		serializer.startTag("", propertyName);
		serializer.text(value);
		serializer.endTag("", propertyName);
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
	
	public static void submitMessage(String route, String message) throws RegistrationFailedException {
		HttpClient httpclient = new DefaultHttpClient();  
		HttpPost httppost = new HttpPost(REG_URL + "/" + route);  

		try {  
			  
			
			StringEntity en = new StringEntity(message);
			en.setContentType("text/xml");
			httppost.setEntity(en);

			// Execute HTTP Post Request  
			HttpResponse response = httpclient.execute(httppost);  

		} catch (ClientProtocolException e) {  
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new RegistrationFailedException (e.toString());
		} catch (IOException e) {  
			// TODO Auto-generated catch block
			throw new RegistrationFailedException (e.toString());
		}
	}
	
}
