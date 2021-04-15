# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

 -optimizationpasses 5
 -dontusemixedcaseclassnames
 -dontskipnonpubliclibraryclasses
 -dontpreverify
 -verbose
 -optimizations !code/simplification/arithmetic,!code/simplification/cast,!field/*,!class/merging/*
 -keepattributes *Annotation*,InnerClasses,Signature,EnclosingMethod
 -renamesourcefileattribute SourceFile
 -keepattributes SourceFile,LineNumberTable
 -dontnote android.support.**
 -dontwarn android.support.**

 -keep public class * extends android.app.Activity
 -keep public class * extends androidx.multidex.MultiDexApplication
 -keep public class * extends android.app.Service
 -keep public class * extends android.content.BroadcastReceiver
 -keep public class * extends android.content.ContentProvider
 -keep public class * extends android.preference.Preference

 -keepclasseswithmembernames class * {
     native <methods>;
 }

 -keepclassmembers enum * {
     public static **[] values();
     public static ** valueOf(java.lang.String);
 }

 -keep class * implements android.os.Parcelable {
     public static final android.os.Parcelable$Creator *;
 }

  #assume no side effects:删除android.util.Log输出的日志
  -assumenosideeffects class android.util.Log {
      public static *** v(...);
      public static *** d(...);
      public static *** i(...);
      public static *** w(...);
      public static *** e(...);
  }

 -keep class com.luoye.bzmedia.bean.** {*;}
 -keep class com.luoye.bzmedia.opengl.** {*;}
 -keep class com.luoye.bzmedia.player.** {*;}
 -keep class com.luoye.bzmedia.BZMedia.** {
       <fields>;
       <methods>;
   }
 -keep class com.luoye.bzmedia.BZMedia** {*;}


