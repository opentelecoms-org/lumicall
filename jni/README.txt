
The JNI code is built and automatically installed to libs/
by running the ndk-build utility.

If you have unpacked the NDK in ~/android/android-ndk-r7, you
could run the following command to build:

    ~/android/android-ndk-r7/ndk-build

By default, Application.mk defaults the APP_ABI to build
for the following target ABIs:

   armeabi-v7a    Most modern/high-end phones
   armeabi        Low cost phones (e.g. Samsung Galaxy Mini)
   x86

Some of the codecs (e.g. G.729) only build on some of the
ABIs however.

To build for a single ABI:

    ~/android/android-ndk-r7/ndk-build APP_ABI=armeabi

and to build for x86:

    ~/android/android-ndk-r7/ndk-build APP_ABI=x86

Look in libs/ to see what architectures were built.  Note
that if you build for a subset of ABIs, ndk-build will delete
the objects that were built for other ABIs on previous
builds.

You can also append NDK_DEBUG=1 to the ndk-build command
if necessary

