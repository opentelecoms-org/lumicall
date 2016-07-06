package org.lumicall.android.sip;

import java.util.List;
import java.util.Vector;

import android.content.Context;
import android.database.SQLException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.omnidial.harvest.ENUMCandidateHarvester;

import org.lumicall.android.db.ENUMSuffix;
import org.lumicall.android.db.LumicallDataSource;

public class LumicallENUMCandidateHarvester extends ENUMCandidateHarvester {
	
	private static final Logger logger = LoggerFactory.getLogger(LumicallENUMCandidateHarvester.class);

	private Context context;
	
	public LumicallENUMCandidateHarvester(Context context) {
		this.context = context;
	}

	@Override
	protected boolean isOnline() {
		return ENUMUtil.updateNotification(context);
	}

	@Override
	protected List<String> getSuffixes() {
		List<String> suffixes = new Vector<String>();
		try {
			LumicallDataSource ds = new LumicallDataSource(context);
			ds.open();
			List<ENUMSuffix> _suffixes = ds.getENUMSuffixes();
			ds.close();
			for(ENUMSuffix s : _suffixes) {
				suffixes.add(s.getSuffix());
			}
		} catch (SQLException e) {
			logger.warn("failed to load ENUM suffixes from DB", e);
			return null;
		}
		return suffixes;
	}

}
