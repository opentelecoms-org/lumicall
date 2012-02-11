package org.zoolu.sdp;

import org.zoolu.tools.Parser;

public class CryptoField extends AttributeField {
	
	final static String FIELD_NAME = "crypto";

	public CryptoField() {
		super(FIELD_NAME);
	}

	public CryptoField(int seq, String suite, String key) {
		super(FIELD_NAME, String.format("%d %s inline:%s", seq, suite, key));
	}

	public CryptoField(SdpField sf) {
		super(sf);
	}
	
	public int getSeq() {
		return Integer.parseInt(new Parser(getAttributeValue()).getString());
	}
	
	public String getSuite() {
		return new Parser(getAttributeValue()).skipString().getString();
	}
	
	public String getKey() {
		String keyData = new Parser(getAttributeValue()).skipString().skipString().getString();
		String prefix = "inline:";
		if(!keyData.startsWith(prefix))
			throw new RuntimeException("Can only handle keys that begin with `inline'");
		return keyData.substring(prefix.length());
	}

}
