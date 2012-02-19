package org.sipdroid.sipua.ui;

public enum InCallAudioPreference {
	
	DETECT,   // will use speaker if docked
	FORCE_SPEAKER;  // will always use speaker
	
	static InCallAudioPreference parseString(String audioMode) {
		for(InCallAudioPreference m : InCallAudioPreference.values()) {
			if(m.toString().equals(audioMode.toUpperCase()))
				return m;
		}
		return null;
	}
	
}
