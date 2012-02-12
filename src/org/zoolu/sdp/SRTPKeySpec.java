package org.zoolu.sdp;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;

import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

import org.opentelecoms.util.Base64;

public class SRTPKeySpec {
	
	byte[] master;
	byte[] salt;
	
	// default sizes in bits
	final static int DEFAULT_KEY_SIZE = 128;
	final static int DEFAULT_SALT_SIZE = 112;
	
	// from RFC 3711
	//  *** THIS CODE IS COMMENTED TO AVOID USING IT BY MISTAKE ***
	//  These keys are only suitable for testing, as they are published in the RFC
	/* final static String SAMPLE_KEY = "E1F97A0D3E018BE0D64FA32C06DE4139";
	final static String SAMPLE_SALT = "0EC675AD498AFEEBB6960B3AABE6";
	
	public static SRTPKeySpec getSampleKeySpec() {
		return new SRTPKeySpec(hexToBytes(SAMPLE_KEY), hexToBytes(SAMPLE_SALT));
	} */
	
	public static byte[] makeKey(int bits) throws NoSuchAlgorithmException {
	    KeyGenerator kgen = KeyGenerator.getInstance("AES");
	    kgen.init(bits);
	    SecretKey key = kgen.generateKey();
	    return key.getEncoded();
	}
	
	public static SRTPKeySpec generate() {
		try {
			SRTPKeySpec _key = new SRTPKeySpec(
				makeKey(DEFAULT_KEY_SIZE),
				makeKey(DEFAULT_SALT_SIZE));
			return _key;
		} catch (NoSuchAlgorithmException ex) {
			throw new RuntimeException("Failed to create master key/salt for SRTP", ex);
		}
	}
	
	public SRTPKeySpec(String sKeySpec) {
		if(sKeySpec.length() < 40) {
			throw new RuntimeException("SRTP key spec too short");
		}
		try {
			byte[] _keySpec = Base64.decode(sKeySpec.substring(0, 40));
			master = copyBytes(_keySpec, 0, 16);
			salt = copyBytes(_keySpec, 16, 14);
			
		} catch (IOException e) {
			throw new RuntimeException("Base64 decoding of SRTP key spec failed");
		}
	}
	
	public SRTPKeySpec(byte[] master, byte[] salt) {
		this.master = copyBytes(master, 0, 16);
		this.salt = copyBytes(salt, 0, 14);
	}
	
	protected byte[] copyBytes(byte[] buf, int offset, int len) {
		byte[] _buf = new byte[len];
		for(int i = 0; i < len; i++) {
			_buf[i] = buf[offset + i];
		}
		return _buf;
	}
	
	public byte[] getMaster() {
		return master;
	}
	
	public byte[] getSalt() {
		return salt;
	}
	
	@Override
	public String toString() {
		byte[] _keySpec = new byte[30];
		for(int i = 0; i < 30; i++) {
			if(i < 16)
				_keySpec[i] = master[i];
			else
				_keySpec[i] = salt[i - 16];
		}
		return Base64.encodeBytes(_keySpec);
		
	}
	
	final static String hexDigits = "0123456789abcdef";
	static byte[] hexToBytes(String s) {
		String _s = s.toLowerCase();
		byte[] buf = new byte[s.length() / 2];
		for(int i = 0; i < buf.length; i++) {
			int pos = i * 2;
			int hi = hexDigits.indexOf(_s.charAt(pos));
			int lo = hexDigits.indexOf(_s.charAt(pos+1));
			buf[i] = (byte)(hi * 16 + lo);
		}
		return buf;
	}
	
}
