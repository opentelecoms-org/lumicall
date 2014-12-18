#!/bin/bash

URL=https://raw.githubusercontent.com/musalbas/mcc-mnc-table/master/mcc-mnc-table.csv

TMPFILE=`mktemp`

OUT=res/raw/country_codes.csv
MCC=res/raw/mcc.csv

wget -O - $URL | grep -v ^MCC > $TMPFILE

cat $TMPFILE | awk -v OFS=, -F"," '{print $6,$5,$7}' | sort | uniq > $OUT

cat $TMPFILE | awk -v OFS=, -F"," '{print $5,$1}' | sort | uniq > $MCC

