package org.lumicall.android.sip;

public class CountryData {
	
	private String name;
	private String isoCountryCode;
	private String ituCountryCode;
	
	public CountryData(String _name, String _isoCountryCode, String _ituCountryCode) {
		setName(_name);
		setIsoCountryCode(_isoCountryCode);
		setItuCountryCode(_ituCountryCode);
	}
	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getIsoCountryCode() {
		return isoCountryCode;
	}
	
	public void setIsoCountryCode(String isoCountryCode) {
		this.isoCountryCode = isoCountryCode;
	}
	
	public String getItuCountryCode() {
		return ituCountryCode;
	}
	
	public void setItuCountryCode(String ituCountryCode) {
		this.ituCountryCode = ituCountryCode;
	}
	
	@Override
	public String toString() {
		return getName() + " (+" + getItuCountryCode() + ")";
	}

}
