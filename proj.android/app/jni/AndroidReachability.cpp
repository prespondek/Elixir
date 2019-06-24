#include "AndroidReachability.h"
#include <jni.h>
#include "platform/android/jni/JniHelper.h"

const char* const CLASS_NAME = "com/bridge/network/ReachabilityBridge";


bool AndroidReachability::checkReachabilityForInternetConnection ()
{
	CCLOG("AndroidReachability::checkReachabilityForInternetConnection");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "isActivityConnected", "()Z")) {
		bool result = (t.env->CallStaticBooleanMethod(t.classID, t.methodID) == JNI_TRUE);
		t.env->DeleteLocalRef(t.classID);
		return result;
	} else {
		return false;
	}
}

extern "C" {
	JNIEXPORT void JNICALL Java_com_bridge_network_ReachabilityBridge_ReachabilityChanged(JNIEnv* env, jclass clazz, jboolean connected)
	{
		CCLOG("Java_com_bridge_network_ReachabilityBridge_ReachabilityChanged");
		Reachability* net_mon = Reachability::getInstance();
		net_mon->reachabilityStatusChanged(ReachabilityStatus(connected));
	}
}
