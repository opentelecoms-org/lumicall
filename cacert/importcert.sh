#!/bin/bash

if [ -z $1 ]; then
  echo "Usage: importcert.sh <CA cert PEM file>"
  exit 1
fi

CACERT=$1
BCJAR=/opt/bouncycastle/bcprov-jdk16-146.jar
KEYTOOL=/opt/jdk1.6/bin/keytool

TRUSTSTORE=mytruststore.bks
ALIAS=`openssl x509 -inform PEM -subject_hash -noout -in $CACERT`

if [ -f $TRUSTSTORE ]; then
    rm $TRUSTSTORE || exit 1
fi

echo "Adding certificate to $TRUSTSTORE..."
$KEYTOOL -import -v -trustcacerts -alias $ALIAS \
      -file $CACERT \
      -keystore $TRUSTSTORE -storetype BKS \
      -providerclass org.bouncycastle.jce.provider.BouncyCastleProvider \
      -providerpath $BCJAR \
      -storepass secret

echo "" 
echo "Added '$CACERT' with alias '$ALIAS' to $TRUSTSTORE..."


