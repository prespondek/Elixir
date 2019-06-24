#include "AndroidPlatformUtils.h"
#include "platform/android/jni/JniHelper.h"
#include <jni.h>


const char* const CLASS_NAME = "com/bridge/platform/util/PlatformUtilBridge";

AndroidPlatformUtils::AndroidPlatformUtils()
{
}

AndroidPlatformUtils::~AndroidPlatformUtils()
{
}


void AndroidPlatformUtils::downloadUrl ( const std::string& url, const std::string& path, const std::function<void(const std::string& url,const std::string& path, bool success)>& func )
{
	CCLOG("PlatformUtils::downloadUrl");
	auto iter = _func_list.insert(std::pair<std::string, std::function<void(const std::string& url,const std::string& path, bool success)>>(path, func));
	if (iter.second == false) return;

	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "downloadUrl", "(Ljava/lang/String;Ljava/lang/String;)V")) {
		jstring jpath = t.env->NewStringUTF(path.c_str());
		jstring jurl = t.env->NewStringUTF(url.c_str());
		t.env->CallStaticVoidMethod(t.classID, t.methodID, jurl, jpath);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(jpath);
		t.env->DeleteLocalRef(jurl);
	}
}
void AndroidPlatformUtils::createUUID ( std::string& uuid )
{
	CCLOG("PlatformUtils::createUUID");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "createUUID", "()Ljava/lang/String;")) {
		jstring jobj = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
		const char *js = t.env->GetStringUTFChars(jobj, NULL);
		uuid = js;
		t.env->ReleaseStringUTFChars(jobj, js);
		t.env->DeleteLocalRef(t.classID);
	}
}
void AndroidPlatformUtils::getBuildNumber ( std::string& out_version, std::string& out_build )
{
	CCLOG("PlatformUtils::getBuildNumber");
		cocos2d::JniMethodInfo t;
		if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getVersionNumber", "()Ljava/lang/String;")) {
			jstring jobj = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
			const char *js = t.env->GetStringUTFChars(jobj, NULL);
			out_version = js;
			t.env->ReleaseStringUTFChars(jobj, js);
			t.env->DeleteLocalRef(t.classID);
		}
		if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getBuildNumber", "()Ljava/lang/String;")) {
			jstring jobj = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
			const char *js = t.env->GetStringUTFChars(jobj, NULL);
			out_build = js;
			t.env->ReleaseStringUTFChars(jobj, js);
			t.env->DeleteLocalRef(t.classID);
		}
}


void AndroidPlatformUtils::downloadCallback( const std::string& url, const std::string& path, bool success)
{
	auto iter = _func_list.find(path);
	if (iter != _func_list.end()) {
		iter->second(url,path,success);
	}
}

bool AndroidPlatformUtils::init()
{
	return true;
}

extern "C" {

	JNIEXPORT void JNICALL Java_com_bridge_platform_util_PlatformUtilBridge_downloadComplete (JNIEnv* env, jclass clazz, jstring jurl, jstring jpath, jboolean jsuccess)
	{
		CCLOG("Java_com_bridge_platform_util_PlatformUtilBridge_downloadComplete");
		AndroidPlatformUtils* util = static_cast<AndroidPlatformUtils*>(PlatformUtils::getInstance());
	    std::string url = cocos2d::JniHelper::jstring2string(jurl);
	    std::string path = cocos2d::JniHelper::jstring2string(jpath);
		util->downloadCallback(url,path,jsuccess);
	}

}

