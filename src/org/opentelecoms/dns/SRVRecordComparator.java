package org.opentelecoms.dns;

import java.util.Comparator;
import java.util.Random;

import org.xbill.DNS.SRVRecord;

/* Compares SRV records, so that the records that should be tried first
 * will be `less than' the records to try later.
 * Where two records have equal priority, ordering is non-deterministic,
 * adhering to the `weighting' mechanism
 * 
 * This is a simple hack that works when there are equally weighted load-balanced servers,
 * but as it doesn't look at the full set of SRV records, the weighting algorithm doesn't respect
 * larger sets with varying weights
 */
public class SRVRecordComparator implements Comparator<SRVRecord> {
	
	Random random = new Random();

	@Override
	public int compare(SRVRecord arg0, SRVRecord arg1) {
		
		if(arg0.getPriority() != arg1.getPriority())
			return arg0.getPriority() - arg1.getPriority();
		
		double total = arg0.getWeight() + arg1.getWeight();
		double weight = arg0.getWeight() / total;
		
		if(random.nextDouble() > weight)
			return 1;
		
		return -1;
	}

}
