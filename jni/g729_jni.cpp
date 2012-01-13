/*
 * Copyright (C) 2009 The Sipdroid Open Source Project
 * 
 * This file is part of Sipdroid (http://www.sipdialer.org)
 * 
 * Sipdroid is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this source code; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <ctype.h>
#include <jni.h>
#include <android/log.h>

extern "C" {
#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"

#include "g729a.h"
}

/*encoder variable for open*/
Word32 encodersize;
void *hEncoder;
/*decoder variable for open*/
Word32 decodersize;
void *hDecoder;

/*Default bit streem size*/
#define	BITSTREAM_SIZE	10
/*Fixed rtp header size*/
#define	RTP_HDR_SIZE	12

#define BLOCK_LEN       160

static int codec_open = 0;

static JavaVM *gJavaVM;
const char *kInterfacePath = "org/sipdroid/pjlib/g729";


extern "C"
JNIEXPORT jint JNICALL Java_org_sipdroid_codecs_G729_open
  (JNIEnv *env, jobject obj) {
	//int ret;
	Flag en;
	Flag de;

	if (codec_open++ != 0)
		return (jint)0;

	/*--------------------------------------------------------------------------*
	 * Initialization of the coder.                                             *
	 *--------------------------------------------------------------------------*/

	encodersize = g729a_enc_mem_size();
	hEncoder = calloc(1, encodersize * sizeof(UWord8));

	if (hEncoder == NULL)
	  {
	    printf("Cannot create encoder\n");
	    exit(2);
	  }
	en=g729a_enc_init(hEncoder);
	if(en == 0)
		  {
		    printf("Cannot create encoder\n");
		    exit(2);
		  }
	/*-----------------------------------------------------------------*
	 *           Initialization of decoder                             *
	 *-----------------------------------------------------------------*/

	decodersize = g729a_dec_mem_size();
	hDecoder = calloc(1, decodersize * sizeof(UWord8));
	  if (hDecoder == NULL)
	  {
	    printf("Cannot create decoder\n");
	    exit(2);
	  }
	  de=g729a_dec_init(hDecoder);
	  if (de == 0)
	  		  {
	  		    printf("Cannot create decoder\n");
	  		    exit(2);
	  		  }

	return (jint)0;
}

extern "C"
JNIEXPORT jint JNICALL Java_org_sipdroid_codecs_G729_encode
    (JNIEnv *env, jobject obj, jshortArray lin, jint offset, jbyteArray encoded, jint size) {

Word16  speech[L_FRAME];
UWord8  serial[BITSTREAM_SIZE];

int i;

int frsz=L_FRAME;

unsigned int lin_pos = 0;

		if (!codec_open)
			return 0;
			
		if(hEncoder == NULL)
			return 0;

for (i = 0; (i + L_FRAME) <= size; i+=frsz)
	 {
		env->GetShortArrayRegion(lin, offset + i,frsz, speech);
		g729a_enc_process(hEncoder, speech, (unsigned char*)serial);
		env->SetByteArrayRegion(encoded, RTP_HDR_SIZE+ lin_pos, BITSTREAM_SIZE, (jbyte *)serial);
		lin_pos += BITSTREAM_SIZE;
	 }

    return (jint)lin_pos;
}

extern "C"
JNIEXPORT jint JNICALL Java_org_sipdroid_codecs_G729_decode
    (JNIEnv *env, jobject obj, jbyteArray encoded, jshortArray lin, jint size) {

		Word16  synth[BLOCK_LEN];

		jbyte  serial[BLOCK_LEN];          /* Serial stream               */


		  unsigned int lin_pos = 0;

		  jbyte i;

		  int len=80;

		  if (!codec_open)
			return 0;
			
		if(hDecoder == NULL)
			return 0;

		  for (i=0; i + BITSTREAM_SIZE <= size; i=i+BITSTREAM_SIZE)
			  {
				  //env->GetByteArrayRegion(encoded, RTP_HDR_SIZE, size, serial);
			      env->GetByteArrayRegion(encoded, i+RTP_HDR_SIZE, BITSTREAM_SIZE,serial);

				  g729a_dec_process(hDecoder, (unsigned char*)serial, synth, 0);

				  //env->SetShortArrayRegion(lin, 0, size,synth);
				  //env->SetShortArrayRegion(lin, lin_pos, size,synth);
				  env->SetShortArrayRegion(lin, lin_pos, len,synth);
				  lin_pos = lin_pos + len;
			  }
		return (jint)lin_pos;
}


extern "C"
JNIEXPORT void JNICALL Java_org_sipdroid_codecs_G729_close
    (JNIEnv *env, jobject obj) {

	if (--codec_open != 0)
		return;

	/*encoder closed*/
	g729a_enc_deinit(hEncoder);
	free(hEncoder);
	hEncoder = NULL;
	/*decoder closed*/
	g729a_dec_deinit(hDecoder);
	free(hDecoder);
	hDecoder = NULL;

}
