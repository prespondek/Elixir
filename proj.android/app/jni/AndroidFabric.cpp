#include "external/Fabric.h"
#include "cocos2d.h"
#include "platform/android/jni/JniHelper.h"

const char* const CLASS_NAME = "com/bridge/fabric/FabricBridge";

void CFabric::CAnswers::logSignUpWithMethod( const std::string& method, bool success )
{
	CCLOG("AndroidFabric::logSignUpWithMethod");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "logSignUpWithMethod", "(Ljava/lang/String;Z)V")) {
		jstring str = t.env->NewStringUTF(method.c_str());
		t.env->CallStaticVoidMethod(t.classID,t.methodID,str,success);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str);
	}
}

void CFabric::CAnswers::logLoginWithMethod ( const std::string& method, bool success )
{
	CCLOG("AndroidFabric::logLoginWithMethod");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "logLoginWithMethod", "(Ljava/lang/String;Z)V")) {
		jstring str = t.env->NewStringUTF(method.c_str());
		t.env->CallStaticVoidMethod(t.classID,t.methodID,str,success);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str);
	}
}
void CFabric::CAnswers::logInviteWithMethod ( const std::string& method )
{
	CCLOG("AndroidFabric::logInviteWithMethod");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "logInviteWithMethod", "(Ljava/lang/String;)V")) {
		jstring str = t.env->NewStringUTF(method.c_str());
		t.env->CallStaticVoidMethod(t.classID,t.methodID,str);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str);
	}
}
void CFabric::CAnswers::logPurchaseWithPrice ( float price,
                                  const std::string& currency,
                                  bool success,
                                  const std::string& itemName,
                                  const std::string& itemType,
                                  const std::string& itemId )
{
	CCLOG("AndroidFabric::logPurchaseWithPrice");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "logPurchaseWithPrice", "(FLjava/lang/String;ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V")) {
		jstring var1 = t.env->NewStringUTF(currency.c_str());
		jstring var2 = t.env->NewStringUTF(itemName.c_str());
		jstring var3 = t.env->NewStringUTF(itemType.c_str());
		jstring var4 = t.env->NewStringUTF(itemId.c_str());

		t.env->CallStaticVoidMethod(t.classID,t.methodID,price,var1,success,var2,var3,var4);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(var1);
		t.env->DeleteLocalRef(var2);
		t.env->DeleteLocalRef(var3);
		t.env->DeleteLocalRef(var4);
	}
}
void CFabric::CAnswers::logLevelStart ( const std::string& method )
{
	CCLOG("AndroidFabric::logLevelStart");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "logLevelStart", "(Ljava/lang/String;)V")) {
		jstring str = t.env->NewStringUTF(method.c_str());
		t.env->CallStaticVoidMethod(t.classID,t.methodID,str);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str);
	}
}
void CFabric::CAnswers::logLevelEnd ( const std::string& method, int score, bool success )
{
	CCLOG("AndroidFabric::logLevelEnd");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "logLevelEnd", "(Ljava/lang/String;IZ)V")) {
		jstring str = t.env->NewStringUTF(method.c_str());
		t.env->CallStaticVoidMethod(t.classID,t.methodID,str,score,success);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str);
	}
}
