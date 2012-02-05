package org.opentelecoms.dns;

import java.net.Inet4Address;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.TreeSet;
import java.util.Vector;

import org.xbill.DNS.ARecord;
import org.xbill.DNS.ExtendedResolver;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.Resolver;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

import android.util.Log;

public class SRVRecordHelper extends Vector<InetSocketAddress> {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = -2656070797887094655L;
	final private String TAG = "SRVRecordHelper";
	
	public SRVRecordHelper(String service, String protocol, String domain, int defaultPort) {
		String mDomain = "_" + service + "._" + protocol + "." + domain;
		
		TreeSet<SRVRecord> srvRecords = new TreeSet<SRVRecord>(new SRVRecordComparator());
		Vector<ARecord> aRecords = new Vector<ARecord>();
		
		try {
			Resolver resolver = new ExtendedResolver();
			resolver.setTimeout(2);
			Lookup lookup = new Lookup(mDomain, Type.SRV);
			lookup.setResolver(resolver);
			Vector<Record> _records = new Vector<Record>();
			Record[] records = lookup.run();
			
			if (records != null)
				for(Record record : records)
					_records.add(record);
			
			if(_records.size() == 0) {
				lookup = new Lookup(domain, Type.A);
				records = lookup.run();
				if (records != null)
					for(Record record : records)
						_records.add(record);
			}

			for (Record record : _records) {
				if(record instanceof SRVRecord) {
					srvRecords.add((SRVRecord)record);
				}
				if(record instanceof ARecord) {
					aRecords.add((ARecord)record);
				}
			}
		} catch (Exception ex) {
			Log.w(TAG, "Exception during DNS lookup: ", ex);
		}

		for(SRVRecord srvRecord : srvRecords) {
			add(new InetSocketAddress(srvRecord.getTarget().toString(), srvRecord.getPort()));
		}
		
		if(defaultPort > 0 && size() == 0) {
			for(ARecord record : aRecords) {
				add(new InetSocketAddress(record.getName().toString(), defaultPort));
			}
		}
		
	}

}
