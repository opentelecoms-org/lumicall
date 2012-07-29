

cd jni && ln -s . jni

To build for armeabi-v7a (required for G729):

~/android/android-ndk-r7/ndk-build  APP_ABI=armeabi-v7a  NDK_DEBUG=1 

Must copy output files to libs:

rsync -avr libs/ ../libs/

and then commit to git potentially


