//
//  IOSFacebook.cpp
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#include "AndroidFacebook.h"
#include <jni.h>

AndroidFacebook::AndroidFacebook() :
_req_count(0)
{

}

AndroidFacebook::~AndroidFacebook()
{

}

AndroidFacebook* AndroidFacebook::get ()
{
	return static_cast<AndroidFacebook*>(Facebook::getInstance());
}

void AndroidFacebook::userDataCallback ( const std::map<std::string,std::string>& user_data )
{
	Facebook::userDataCallback(user_data);
}

void AndroidFacebook::errorCallback ( int code )
{
	Facebook::errorCallback(code);
}

void AndroidFacebook::friendDataCallback ( const std::vector<std::map<std::string,std::string>>& friends_data )
{
	Facebook::friendDataCallback(friends_data);
}

void AndroidFacebook::userLoggedIn ( bool success )
{
	Facebook::userLoggedIn(success);
}

bool AndroidFacebook::init ( )
{
	return true;
}
void AndroidFacebook::logIn ( )
{
	CCLOG("AndroidFacebook::logIn");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "logIn");
}
void AndroidFacebook::logOut ( )
{
	CCLOG("AndroidFacebook::logOut");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "logOut");
}
void AndroidFacebook::postStatusUpdate ( const std::string& caption )
{

}

void AndroidFacebook::postLink               ( const std::string& link )
{
	CCLOG("AndroidFacebook::postLink");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "postLink", "(Ljava/lang/String;)V")) {
		jstring japp_link = t.env->NewStringUTF(link.c_str());
		t.env->CallStaticVoidMethod(t.classID, t.methodID, japp_link);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(japp_link);
	}
}

void AndroidFacebook::openAppInviteDialog ( const std::string& app_link, const std::string& img_url )
{
	CCLOG("AndroidFacebook::openAppInviteDialog");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "openAppInviteDialog", "(Ljava/lang/String;Ljava/lang/String;)V")) {
		jstring japp_link = t.env->NewStringUTF(app_link.c_str());
		jstring jimg_url = t.env->NewStringUTF(img_url.c_str());
		t.env->CallStaticVoidMethod(t.classID, t.methodID, japp_link, jimg_url);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(japp_link);
		t.env->DeleteLocalRef(jimg_url);
	}
}
void AndroidFacebook::sendGameRequest        ( FBGameRequestType type,
                                               const std::string& message,
                                          	   const std::string& title,
                                          	   const std::string& recipient )
{
	CCLOG("AndroidFacebook::sendGameRequest");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "sendGameRequest", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V")) {
		jstring jmessage = t.env->NewStringUTF(message.c_str());
		jstring jtitle = t.env->NewStringUTF(title.c_str());
		jstring jrecipient = t.env->NewStringUTF(recipient.c_str());
		t.env->CallStaticVoidMethod(t.classID, t.methodID, (int)type, jmessage, jtitle, jrecipient);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(jmessage);
		t.env->DeleteLocalRef(jtitle);
		t.env->DeleteLocalRef(jrecipient);
	}
}
void AndroidFacebook::deleteGameRequest ( const std::shared_ptr<FacebookGameRequest>& req )
{
	CCLOG("AndroidFacebook::deleteGameRequest");
	CCASSERT ((_game_request && _req_count == 0), "can't do two delete requests at a time");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "deleteGameRequest", "([Ljava/lang/String;)V")) {
		_game_request = req;
		jobjectArray result;
		int len = req->req_ids.size();
		result = t.env->NewObjectArray(len,t.env->FindClass("java/lang/String"),0);
		for( int i = 0; i < len; i++ )
		{
			jstring str = t.env->NewStringUTF(req->req_ids[i].c_str());
			t.env->SetObjectArrayElement(result,i,str);
			t.env->DeleteLocalRef(str);
		}
		t.env->CallStaticVoidMethod(t.classID, t.methodID,result);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(result);
	}
}
bool AndroidFacebook::parseURL ( const std::string& url )
{
	CCLOG("AndroidFacebook::parseURL");
	return Facebook::parseURL (url);
}
void AndroidFacebook::getUserData ( )
{
	CCLOG("AndroidFacebook::getUserData");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "getUserData");
}

void AndroidFacebook::refreshAccessToken( )
{
	CCLOG("AndroidFacebook::refreshAccessToken");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "refreshAccessToken");
}

void AndroidFacebook::getFriendData ( )
{
	CCLOG("AndroidFacebook::getFriendData");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "getFriendData");
}

void AndroidFacebook::getGameRequests ( const std::string& app_id )
{
	CCLOG("AndroidFacebook::getGameRequests");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME,"getGameRequests",app_id.c_str());
}
bool AndroidFacebook::isLoggedIn ( ) const
{
	CCLOG("AndroidFacebook::isLoggedIn");
	return cocos2d::JniHelper::callStaticBooleanMethod(CLASS_NAME,"isLoggedIn");
}

void AndroidFacebook::gameRequestRemoved				( std::string req )
{
	_req_count++;
}

void AndroidFacebook::gameRequestRemoveComplete( )
{
	gameRequestDeletedCallback( _req_count == _game_request->req_ids.size(), _game_request );
	_req_count = 0;
	_game_request = nullptr;
}


extern "C" {

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_recieveGameRequests ( JNIEnv* env, jclass clazz, jobjectArray array, jboolean success )
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_recieveGameRequests");
        std::map<std::string, std::shared_ptr<FacebookGameRequest>> reqs;
		int rows = env->GetArrayLength(array);
		for(int i = 0; i < rows; i++)
		{
		    jobjectArray currentRow = (jobjectArray)env->GetObjectArrayElement(array, i);
			int columns = env->GetArrayLength(currentRow);
	        FacebookGameRequest* game_req = new FacebookGameRequest();
	        std::string message; std::string from_id; std::string data;
	        std::string from_name; std::string req_id;
		    for(int j = 0; j < columns; j++)
		    {
		        jstring line = (jstring)env->GetObjectArrayElement(currentRow, j);
		        const char *rawString = env->GetStringUTFChars(line, 0);
		        switch (j) {
		        	case 0 : message = rawString; break;
		        	case 1 : req_id = rawString; break;
					case 2 : data = rawString; break;
					case 3 : from_id = rawString; break;
		        	case 4 : from_name = rawString; break;
		        }
		        env->ReleaseStringUTFChars(line, rawString);
		    }
		    std::string key = message + from_id;
		    auto iter = reqs.find(key);
		    if (iter == reqs.end()) {
		    	FacebookGameRequest* game_req = new FacebookGameRequest();
		    	game_req->message = message;
				game_req->type = (FBGameRequestType)atoi(data.c_str());
		    	game_req->from_id = from_id;
		    	game_req->from_name = from_name;
		    	game_req->req_ids.push_back(req_id);
		    	reqs.insert(std::pair<std::string, std::shared_ptr<FacebookGameRequest>>
		    			(key, std::shared_ptr<FacebookGameRequest>(game_req)));
		    } else {
                iter->second->req_ids.push_back(req_id);
            }
		}
		AndroidFacebook::get()->gameRequestsCallback(success, reqs);
	}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_userLoggedIn ( JNIEnv* env, jclass clazz, jboolean success )
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_userLoggedIn");
		AndroidFacebook* fb = AndroidFacebook::get();
		fb->userLoggedIn(success);
	}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_recieveFriendData ( JNIEnv* env, jclass clazz, jobjectArray reqs, jboolean success)
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_recieveFriendData");
        std::vector<std::map<std::string,std::string>> friends_data;
        if (!success) {
        	AndroidFacebook::get()->friendDataCallback(friends_data);
        	return;
        }

        int rows = env->GetArrayLength(reqs);
        for(int i = 0; i < rows; i++)
        {
        	jobjectArray currentRow = (jobjectArray)env->GetObjectArrayElement(reqs, i);
        	int columns = env->GetArrayLength(currentRow);
			std::map<std::string, std::string> friend_data;
			for(int j = 0; j < columns; j++)
			{
				jstring line = (jstring)env->GetObjectArrayElement(currentRow, j);
				const char* rawString = env->GetStringUTFChars(line, 0);
				switch (j) {
					case 0: friend_data.insert(std::pair<std::string,std::string>("name", rawString)); break;
					case 1: friend_data.insert(std::pair<std::string,std::string>("id", rawString)); break;
					case 2: friend_data.insert(std::pair<std::string,std::string>("picture", rawString)); break;
				}
				env->ReleaseStringUTFChars(line, rawString);
			}
			friends_data.push_back(friend_data);
        }
        AndroidFacebook::get()->friendDataCallback(friends_data);
	}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_recieveUserData (JNIEnv* env, jclass clazz, jobjectArray data, jboolean success)
	{
		CCLOG("Java_com_brige_facebook_FacebookBridge_recieveUserData");
        std::map<std::string,std::string> user_data;
    	if (!success) {
    		AndroidFacebook::get()->userDataCallback(user_data);
    	    return;
    	}
    	int columns = env->GetArrayLength(data);
		for(int j = 0; j < columns; j++)
		{
			jstring line = (jstring)env->GetObjectArrayElement(data, j);
			const char* rawString = env->GetStringUTFChars(line, 0);
			switch (j) {
				case 0: user_data.insert(std::pair<std::string,std::string>("name", rawString)); break;
				case 1: user_data.insert(std::pair<std::string,std::string>("id", rawString)); break;
				case 2: user_data.insert(std::pair<std::string,std::string>("picture", rawString)); break;
				case 3: user_data.insert(std::pair<std::string,std::string>("permissions", rawString)); break;
			}
			env->ReleaseStringUTFChars(line, rawString);
		}
		AndroidFacebook::get()->userDataCallback(user_data);
	}

	JNIEXPORT bool JNICALL Java_com_bridge_facebook_FacebookBridge_incomingUrl (JNIEnv* env, jclass clazz, jstring jurl)
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_incomingUrl");
	    std::string url = cocos2d::JniHelper::jstring2string(jurl);
	    return AndroidFacebook::get()->parseURL(url);
	}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_errorCallback (JNIEnv* env, jclass clazz, jint code)
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_gameRequestRemoved");
		AndroidFacebook::get()->errorCallback(code);

	}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_gameRequestRemoved (JNIEnv* env, jclass clazz, jstring jreq, jboolean jsuccess)
		{
			CCLOG("Java_com_bridge_facebook_FacebookBridge_gameRequestRemoved");
		    std::string req = cocos2d::JniHelper::jstring2string(jreq);
		    if (jsuccess == true)
		    	AndroidFacebook::get()->gameRequestRemoved(req);

		}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_gameRequestRemoveComplete (JNIEnv* env, jclass clazz)
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_gameRequestRemoveComplete");
		AndroidFacebook::get()->gameRequestRemoveComplete();
	}

	JNIEXPORT void JNICALL Java_com_bridge_facebook_FacebookBridge_gameRequestComplete (JNIEnv* env, jclass clazz, jboolean jsuccess)
	{
		CCLOG("Java_com_bridge_facebook_FacebookBridge_gameRequestComplete");
		AndroidFacebook::get()->gameRequestComplete(jsuccess);
	}

}
