
The JNI code is built and automatically installed to libs/
by running the ndk-build utility.

If you have unpacked the NDK in ~/android/android-ndk-r7, you
could run the following command to build:

    ~/android/android-ndk-r7/ndk-build

By default, Application.mk defaults the ABI to armeabi-v7a.
That is currently required for the G.729 code to build.

To build for legacy armeabi (without attempting to compile G.729):

    ~/android/android-ndk-r7/ndk-build APP_ABI=armeabi

and to build for x86:

    ~/android/android-ndk-r7/ndk-build APP_ABI=x86

Look in libs/ to see what architectures were built.

You can also append NDK_DEBUG=1 to the ndk-build command
if necessary

