#!/bin/bash


# adds a 2nd (3rd, 4th, ...) root to mytruststore.bks

EXTRA_ROOT_ALIAS=$1
EXTRA_ROOT_FILE=$2

keytool -keystore mytruststore.bks \
  -storetype BKS -provider org.bouncycastle.jce.provider.BouncyCastleProvider \
  -providerpath /opt/bouncycastle/bcprov-jdk16-146.jar \
  -storepass secret \
  -importcert -trustcacerts -alias $EXTRA_ROOT_ALIAS -file $EXTRA_ROOT_FILE

