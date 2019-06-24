#include "external/Notifications.h"
#include <jni.h>
#include "cocos2d.h"
#include "platform/android/jni/JniHelper.h"

const char* const CLASS_NAME = "com/bridge/notifications/NotificationBridge";

void Notifications::requestNotificationPremission ()
{
    
}

void Notifications::postLocalNotification ( time_t time, const char* str, const char* sound )
{
	CCLOG("Notifications::postLocalNotification");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "scheduleNotification", "(Ljava/lang/String;J)V")) {
		jstring note = t.env->NewStringUTF(str);
		t.env->CallStaticVoidMethod(t.classID, t.methodID, note, (jlong)time);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(note);
	}
}
void Notifications::cancelLocalNotifications ( )
{
	CCLOG("Notifications::cancelLocalNotifications");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "clearNotifications", "()V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
	}
}
void Notifications::setApplicationIconBadgeNumber (int num)
{
	CCLOG("Notifications::setApplicationIconBadgeNumber");

}

bool Notifications::isNotificationPermitted()
{
	CCLOG("Notifications::isNotificationPermitted");
	return true;
}



