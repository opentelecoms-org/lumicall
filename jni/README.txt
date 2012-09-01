
To build for armeabi  (doesn't build G729):

    ~/android/android-ndk-r7/ndk-build

To build for armeabi-v7a (required for G729):

    ~/android/android-ndk-r7/ndk-build APP_ABI=armeabi-v7a 

Look in libs/ to see what architectures were built.

You can also append NDK_DEBUG=1 to the ndk-build command
if necessary


