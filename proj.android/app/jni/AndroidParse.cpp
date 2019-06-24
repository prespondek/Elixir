#include "AndroidParse.h"
#include "platform/android/jni/JniHelper.h"

AndroidParseWrapperObject::AndroidParseWrapperObject( jobject jobj, JNIEnv* env ) :
_env(nullptr),
_jobj(nullptr)
{
	setJObject(jobj,env);
}

AndroidParseWrapperObject::~AndroidParseWrapperObject()
{
	clearJObject();
}

void AndroidParseWrapperObject::setJObject( jobject obj, JNIEnv *env )
{
	clearJObject( env );
	if ( env == nullptr ) {
		_jobj = obj;
		return;
	}
	_jobj = env->NewGlobalRef(obj);
	_env = env;
}

void AndroidParseWrapperObject::clearJObject( JNIEnv* env )
{
	if ( _jobj ) {
	    JniHelper::getEnv()->DeleteGlobalRef(_jobj);
	}
	_env = nullptr;
	_jobj = nullptr;
}

std::string AndroidParseWrapperObject::getClassName ( )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring str = (jstring)env->CallStaticObjectMethod( wrapper->_jparse_bridge, wrapper->_class_name, _jobj );
	std::string value = cocos2d::JniHelper::jstring2string(str);
	return value;
}

std::string AndroidParseWrapperObject::getObjectId ( )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring str = (jstring)env->CallStaticObjectMethod( wrapper->_jparse_bridge, wrapper->_object_id, _jobj );
	std::string value = cocos2d::JniHelper::jstring2string(str);
	return value;
}

long AndroidParseWrapperObject::getUpdatedAt ( )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	return env->CallStaticLongMethod( wrapper->_jparse_bridge, wrapper->_updated_at, _jobj ) / 1000;
}

long AndroidParseWrapperObject::getCreatedAt ( )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	return long(env->CallStaticLongMethod( wrapper->_jparse_bridge, wrapper->_created_at, _jobj ) / 1000);
}

void AndroidParseWrapperObject::getValue ( const std::string& key, std::set<std::string>& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	if (env->CallBooleanMethod(_jobj, wrapper->_jhas_value, jkey)) {
		jobjectArray obj_array = (jobjectArray)env->CallStaticObjectMethod(wrapper->_jparse_bridge, wrapper->_jload_value_ss, _jobj, jkey);
		if (!obj_array) {
			return;
		}
		int rows = env->GetArrayLength(obj_array);
		for(int i = 0; i < rows; i++)
		{
			jstring line = (jstring)env->GetObjectArrayElement(obj_array, i);
			if (line) {
				const char *rawString = env->GetStringUTFChars(line, 0);
				value.insert(rawString);
				CCLOG("%s",rawString);
				env->ReleaseStringUTFChars(line, rawString);
			} else {
				value.insert("");
			}
		}
	}
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::getValue ( const std::string& key, std::string& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
	JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	if (env->CallBooleanMethod(_jobj, wrapper->_jhas_value, jkey)) {
		jstring str = (jstring)env->CallStaticObjectMethod(wrapper->_jparse_bridge, wrapper->_jload_value_str, _jobj, jkey);
		value = cocos2d::JniHelper::jstring2string(str);
		CCLOG("%s",value.c_str());
	}
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::getValue ( const std::string& key, unsigned int& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
	JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	if (env->CallBooleanMethod(_jobj, wrapper->_jhas_value, jkey)) {
		jint num = env->CallStaticIntMethod(wrapper->_jparse_bridge, wrapper->_jload_value_i, _jobj, jkey);
		value = num;
		CCLOG("%s",CCLanyard_Util::toString(value).c_str());
	}
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::getValue ( const std::string& key, unsigned short& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
	JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	if (env->CallBooleanMethod(_jobj, wrapper->_jhas_value, jkey)) {
		jshort num = env->CallStaticShortMethod(wrapper->_jparse_bridge, wrapper->_jload_value_s, _jobj, jkey);
		value = num;
		CCLOG("%s",CCLanyard_Util::toString(value).c_str());
	}
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::getValue ( const std::string& key, long& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
    if (env->CallBooleanMethod(_jobj, wrapper->_jhas_value, jkey)) {
    	jlong num = env->CallStaticLongMethod(wrapper->_jparse_bridge, wrapper->_jload_value_l, _jobj, jkey);
    	value = num;
		CCLOG("%s",CCLanyard_Util::toString(value).c_str());
    }
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::getValue ( const std::string& key, Date& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
	JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	if (env->CallBooleanMethod(_jobj, wrapper->_jhas_value, jkey)) {
		jlong num = env->CallStaticLongMethod(wrapper->_jparse_bridge, wrapper->_jload_value_d, _jobj, jkey);
		value.setTime(num / 1000);
		CCLOG("%s",CCLanyard_Util::toString(value.getTime()).c_str());
	}
	env->DeleteLocalRef(jkey);
}

void AndroidParseWrapperObject::setValue ( const std::string& key, const std::set<std::string>& value )
{
	AndroidParseWrapper* wrapper = AndroidParseWrapper::get();
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jobjectArray result;
	int len = value.size();
	result = env->NewObjectArray(len, env->FindClass("java/lang/String"), 0);
	uint32_t count = 0;
	for( auto iter : value )
	{
		jstring str = env->NewStringUTF(iter.c_str());
		env->SetObjectArrayElement(result,count,str);
		env->DeleteLocalRef(str);
		count++;
	}
	jstring jkey = env->NewStringUTF(key.c_str());
	env->CallStaticVoidMethod(wrapper->_jparse_bridge, wrapper->_jsave_value_ss, _jobj, jkey, result);
	env->DeleteLocalRef(result);
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::setValue ( const std::string& key, const std::string& value )
{
	AndroidParseWrapper* wrapper = static_cast<AndroidParseWrapper*>(ParseWrapper::getInstance());
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	jstring str = env->NewStringUTF(value.c_str());
	env->CallStaticVoidMethod(wrapper->_jparse_bridge, wrapper->_jsave_value_str, _jobj, jkey, str);
	env->DeleteLocalRef(str);
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::setValue ( const std::string& key, const unsigned int& value )
{
	AndroidParseWrapper* wrapper = static_cast<AndroidParseWrapper*>(ParseWrapper::getInstance());
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	env->CallStaticVoidMethod(wrapper->_jparse_bridge, wrapper->_jsave_value_i, _jobj, jkey, value);
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::setValue ( const std::string& key, const unsigned short& value )
{
	AndroidParseWrapper* wrapper = static_cast<AndroidParseWrapper*>(ParseWrapper::getInstance());
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	env->CallStaticVoidMethod(wrapper->_jparse_bridge, wrapper->_jsave_value_s, _jobj, jkey, value);
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::setValue ( const std::string& key, const long& value )
{
	AndroidParseWrapper* wrapper = static_cast<AndroidParseWrapper*>(ParseWrapper::getInstance());
	JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	env->CallStaticVoidMethod(wrapper->_jparse_bridge, wrapper->_jsave_value_l, _jobj, jkey, value);
	env->DeleteLocalRef(jkey);
}
void AndroidParseWrapperObject::setValue ( const std::string& key, const Date& value )
{
	AndroidParseWrapper* wrapper = static_cast<AndroidParseWrapper*>(ParseWrapper::getInstance());
    JNIEnv *env = cocos2d::JniHelper::getEnv();
	jstring jkey = env->NewStringUTF(key.c_str());
	env->CallStaticVoidMethod(wrapper->_jparse_bridge, wrapper->_jsave_value_d, _jobj, jkey, value.getTime() * 1000);
	env->DeleteLocalRef(jkey);
}

AndroidParseWrapperObject*	AndroidParseWrapperObject::get		( ParseWrapperObject* obj )
{
	return static_cast<AndroidParseWrapperObject*>(obj);
}

const char* const CLASS_NAME = "com/bridge/parse/ParseBridge";

AndroidParseWrapper::AndroidParseWrapper ( )
{
	// caching methods;
    JNIEnv *env = 			cocos2d::JniHelper::getEnv();
    jclass parse_bridge =	env->FindClass(CLASS_NAME);
    jclass parse_object =   env->FindClass("com/parse/ParseObject");
    _jparse_bridge = 		(jclass)env->NewGlobalRef(parse_bridge);
    _jhas_value = 			env->GetMethodID(parse_object, "has", "(Ljava/lang/String;)Z");
    _class_name = 			env->GetStaticMethodID(_jparse_bridge, "getClassName", 	"(Lcom/parse/ParseObject;)Ljava/lang/String;");
    _object_id = 			env->GetStaticMethodID(_jparse_bridge, "getObjectId", 	"(Lcom/parse/ParseObject;)Ljava/lang/String;");
    _updated_at = 			env->GetStaticMethodID(_jparse_bridge, "getUpdatedAt", 	"(Lcom/parse/ParseObject;)J");
    _created_at = 			env->GetStaticMethodID(_jparse_bridge, "getCreatedAt", 	"(Lcom/parse/ParseObject;)J");
    _jload_value_ss = 		env->GetStaticMethodID(_jparse_bridge, "loadArray", 	"(Lcom/parse/ParseObject;Ljava/lang/String;)[Ljava/lang/Object;");
    _jload_value_str = 		env->GetStaticMethodID(_jparse_bridge, "loadString", 	"(Lcom/parse/ParseObject;Ljava/lang/String;)Ljava/lang/String;");
    _jload_value_s = 		env->GetStaticMethodID(_jparse_bridge, "loadShort", 	"(Lcom/parse/ParseObject;Ljava/lang/String;)S");
    _jload_value_i = 		env->GetStaticMethodID(_jparse_bridge, "loadInt", 		"(Lcom/parse/ParseObject;Ljava/lang/String;)I");
    _jload_value_l = 		env->GetStaticMethodID(_jparse_bridge, "loadLong", 		"(Lcom/parse/ParseObject;Ljava/lang/String;)J");
    _jload_value_d = 		env->GetStaticMethodID(_jparse_bridge, "loadDate", 		"(Lcom/parse/ParseObject;Ljava/lang/String;)J");
    _jsave_value_ss = 		env->GetStaticMethodID(_jparse_bridge, "saveArray", 	"(Lcom/parse/ParseObject;Ljava/lang/String;[Ljava/lang/String;)V");
    _jsave_value_str = 		env->GetStaticMethodID(_jparse_bridge, "saveString", 	"(Lcom/parse/ParseObject;Ljava/lang/String;Ljava/lang/String;)V");
    _jsave_value_s = 		env->GetStaticMethodID(_jparse_bridge, "saveShort", 	"(Lcom/parse/ParseObject;Ljava/lang/String;S)V");
    _jsave_value_i = 		env->GetStaticMethodID(_jparse_bridge, "saveInt", 		"(Lcom/parse/ParseObject;Ljava/lang/String;I)V");
    _jsave_value_l = 		env->GetStaticMethodID(_jparse_bridge, "saveLong", 		"(Lcom/parse/ParseObject;Ljava/lang/String;J)V");
    _jsave_value_d = 		env->GetStaticMethodID(_jparse_bridge, "saveDate", 		"(Lcom/parse/ParseObject;Ljava/lang/String;J)V");
	env->DeleteLocalRef(parse_bridge);
	env->DeleteLocalRef(parse_object);
}

AndroidParseWrapper::~AndroidParseWrapper ( )
{
    JNIEnv *env = 		cocos2d::JniHelper::getEnv();
	env->DeleteGlobalRef(_jparse_bridge);
}

void AndroidParseWrapper::loginWithFacebook ( )
{
	CCLOG("AndroidParseWrapper::logInWithFacebook");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "logInWithFacebook");
}

void AndroidParseWrapper::logOut ( )
{
	CCLOG("AndroidParseWrapper::logOut");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "logOut");
	for ( auto i : _delegates) {
		i->parseLogOutCallback();
	}
}

AndroidParseWrapper* AndroidParseWrapper::get ( )
{
	return static_cast<AndroidParseWrapper*>(ParseWrapper::getInstance());
}

void AndroidParseWrapper::loadWithObjectId ( const std::shared_ptr<SaveClass>& obj, const std::string& key )
{
	CCLOG("AndroidParseWrapper::loadWithObjectId");
	cocos2d::JniMethodInfo t;
	jobject jobj = createObject(t, obj->getKey(), key);

	ParseInterfaceObject wrap (obj, std::shared_ptr<AndroidParseWrapperObject>( new AndroidParseWrapperObject (jobj, t.env) ));

	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "fetchObjectFromLocalDatastore", "(Lcom/parse/ParseObject;)V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID, jobj );
		t.env->DeleteLocalRef(t.classID);
	}
	wrap.loadObject();
}

int AndroidParseWrapper::saveWithObjectId ( const std::shared_ptr<SaveClass>& obj, const std::string& key )
{
	CCLOG("AndroidParseWrapper::saveWithObjectId");
	cocos2d::JniMethodInfo t;
	jobject jobj = createObject(t, obj->getKey(), key);
	auto query = makeSaveWrapper(obj,jobj,t.env);
	query->second->saveObject();
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "saveObject", "(ILcom/parse/ParseObject;Z)V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID, query->first, jobj, false );
		t.env->DeleteLocalRef(t.classID);
	}
	return query->first;
}

void AndroidParseWrapper::deleteWithObjectId ( const std::string& table, const std::string& key )
{
	CCLOG("AndroidParseWrapper::deleteWithObjectId");
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "deleteWithObjectId", table.c_str(), key.c_str());
}

int AndroidParseWrapper::saveNewObject ( const std::shared_ptr<SaveClass>& obj )
{
	CCLOG("AndroidParseWrapper::saveNewObject");
	cocos2d::JniMethodInfo t;
	jobject jobj = createObject(t, obj->getKey(), "");
	auto query = makeSaveWrapper(obj,jobj,t.env);
	query->second->saveObject();
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "saveObject", "(ILcom/parse/ParseObject;Z)V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID, query->first, jobj, false );
		t.env->DeleteLocalRef(t.classID);
	}
	return query->first;
}

jobject AndroidParseWrapper::createObject ( JniMethodInfo& t, std::string class_name, std::string obj_id )
{
	CCLOG("AndroidParseWrapper::createObject");
	jobject jobj = nullptr;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "createObject", "(Ljava/lang/String;Ljava/lang/String;)Lcom/parse/ParseObject;")) {
		jstring str1 = t.env->NewStringUTF(class_name.c_str());
		jstring str2 = t.env->NewStringUTF(obj_id.c_str());
		jobj = t.env->CallStaticObjectMethod(t.classID, t.methodID, str1, str2);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str1);
		t.env->DeleteLocalRef(str2);
	}
	return jobj;
}

void AndroidParseWrapper::unpinWithName ( const std::string& key )
{
	CCLOG("AndroidParseWrapper::unpinWithName");

    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "unpinWithName", key.c_str());
}

int AndroidParseWrapper::getFriends ( const std::vector<std::string>& friend_indices, bool fromDatastore )
{
	CCLOG("AndroidParseWrapper::getFriends");
	int query_id = createQuery();
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getFriends", "(I[Ljava/lang/String;Z)V")) {
		jobjectArray result;
		int len = friend_indices.size();
		result = t.env->NewObjectArray(len,t.env->FindClass("java/lang/String"),0);
		uint32_t count = 0;
		for( auto iter : friend_indices )
		{
			jstring str = t.env->NewStringUTF(iter.c_str());
			t.env->SetObjectArrayElement(result,count,str);
			t.env->DeleteLocalRef(str);
			count++;
		}
		t.env->CallStaticVoidMethod(t.classID,t.methodID,query_id,result,fromDatastore);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(result);
	}
	return query_id;
}

int AndroidParseWrapper::getMapScores ( const std::string& obj_id, uint16_t language, uint16_t min, uint16_t max, bool fromDatastore )
{
	CCLOG("AndroidParseWrapper::getMapScores");
	int query_id = createQuery();
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getMapScores", "(ILjava/lang/String;IIIZ)V")) {
		jstring str = t.env->NewStringUTF(obj_id.c_str());
		t.env->CallStaticVoidMethod(t.classID,t.methodID,query_id,str,(int)language,(int)min,(int)max,fromDatastore);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(str);
	}
	return query_id;
}

int AndroidParseWrapper::findObjectWithKey   ( const std::string& table,
                                                    const std::string& key,
                                                    const std::string& value,
                                                    bool fromDataStore )
{
	CCLOG("AndroidParseWrapper::findObjectWithKey");
	int query_id = createQuery();
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "findObjectWithKey", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)V")) {
		jstring jtable = t.env->NewStringUTF(table.c_str());
		jstring jkey = t.env->NewStringUTF(key.c_str());
		jstring jvalue = t.env->NewStringUTF(value.c_str());
		t.env->CallStaticVoidMethod(t.classID, t.methodID, query_id, jtable, jkey, jvalue, fromDataStore);
		t.env->DeleteLocalRef(t.classID);
		t.env->DeleteLocalRef(jtable);
		t.env->DeleteLocalRef(jkey);
		t.env->DeleteLocalRef(jvalue);
	}
	return query_id;
}

int AndroidParseWrapper::fetchUser ( )
{
	CCLOG("AndroidParseWrapper::saveUser");
	cocos2d::JniMethodInfo t;
	jobject jobj = nullptr;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getUserObject", "()Lcom/parse/ParseObject;")) {
		jobj = t.env->CallStaticObjectMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
	}
	int query_id = createQuery();
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "fetchObjectInBackground", "(ILcom/parse/ParseObject;)V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID, query_id, jobj );
		t.env->DeleteLocalRef(t.classID);
	}
	return query_id;
}

int AndroidParseWrapper::saveUser ( const std::shared_ptr<SaveClass>& obj, bool cloud_save )
{
	CCLOG("AndroidParseWrapper::saveUser");
	cocos2d::JniMethodInfo t;
	jobject jobj = nullptr;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getUserObject", "()Lcom/parse/ParseObject;")) {
		jobj = t.env->CallStaticObjectMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
	}
	auto query = makeSaveWrapper(obj, jobj, t.env);
	query->second->saveObject();
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "saveObject", "(ILcom/parse/ParseObject;Z)V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID, query->first, jobj, !cloud_save );
		t.env->DeleteLocalRef(t.classID);
	}
	return query->first;
}

std::shared_ptr<ParseInterfaceObject>
AndroidParseWrapper::loadUser ( const std::shared_ptr<SaveClass>& obj )
{
	CCLOG("AndroidParseWrapper::loadUser");
	cocos2d::JniMethodInfo t;
	jobject jobj = nullptr;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "getUserObject", "()Lcom/parse/ParseObject;")) {
		jobj = t.env->CallStaticObjectMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
	}
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "fetchObjectFromLocalDatastore", "(Lcom/parse/ParseObject;)V")) {
		t.env->CallStaticVoidMethod(t.classID, t.methodID, jobj );
		t.env->DeleteLocalRef(t.classID);
	}
	std::shared_ptr<ParseInterfaceObject> wrap (new ParseInterfaceObject (obj,std::shared_ptr<AndroidParseWrapperObject>(new AndroidParseWrapperObject(jobj, t.env))));
	wrap->loadObject();
	return wrap;
}

std::map<int,std::shared_ptr<ParseInterfaceObject>>::iterator
AndroidParseWrapper::makeSaveWrapper ( const std::shared_ptr<SaveClass>& obj, jobject jobj, JNIEnv* env )
{
	int query_id = incrementQueryId();
	std::shared_ptr<ParseInterfaceObject> wrap (new ParseInterfaceObject (obj,std::shared_ptr<AndroidParseWrapperObject>(new AndroidParseWrapperObject(jobj, env))));
	auto req = _reqs.insert(std::pair<int,std::shared_ptr<ParseInterfaceObject>>(query_id,wrap));
	assert(req.second);
	return req.first;
}

int AndroidParseWrapper::createQuery ( )
{
	int query_id = incrementQueryId();
	_queries.insert(std::pair<int,std::vector<std::shared_ptr<AndroidParseWrapperObject>>>(query_id,std::vector<std::shared_ptr<AndroidParseWrapperObject>>()));
	return query_id;
}

bool AndroidParseWrapper::isLoggedIn ( )
{
	CCLOG("AndroidParseWrapper::isLoggedIn");
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, CLASS_NAME, "isLoggedIn", "()Z")) {
		bool logged_in = (t.env->CallStaticBooleanMethod(t.classID, t.methodID) == JNI_TRUE);
		t.env->DeleteLocalRef(t.classID);
		return logged_in;
	}
	return false;
}

void AndroidParseWrapper::parseLoginCallback ( ParseDelegate::LoginState login, bool is_new )
{
	for ( auto i : _delegates) {
		i->parseLoginCallback(login, is_new);
	}
}

void AndroidParseWrapper::objectSaved ( int query_id, const std::string& name, const std::string& object_id, bool success)
{
	auto iter = _reqs.find(query_id);
	if (iter != _reqs.end()) {
		for ( auto i : _delegates) {
			i->parseObjectSaved( iter->second, query_id, success );
		}
		_reqs.erase(iter);
	}
}

void AndroidParseWrapper::objectLoaded ( int query_id, JNIEnv* env, jobject jobj, bool success)
{
	auto iter = _reqs.find(query_id);
	if (iter != _reqs.end()) {

		std::shared_ptr<ParseInterfaceObject>& obj = iter->second;
		//obj->setJObject(jobj, env);
		obj->loadObject();
		_reqs.erase(iter);
	}
}

void AndroidParseWrapper::queryLoaded ( JNIEnv* env, int query_id, jobject obj )
{
	auto iter = _queries.insert(std::pair<int,std::vector<std::shared_ptr<AndroidParseWrapperObject>>>(query_id,std::vector<std::shared_ptr<AndroidParseWrapperObject>>()));
	iter.first->second.push_back(std::shared_ptr<AndroidParseWrapperObject>(new AndroidParseWrapperObject(obj,env)));
}

void AndroidParseWrapper::queryStarted ( int query_id, const std::string& table, bool success )
{
    for ( auto i : _delegates ) {
    	i->parseQueryStarted(query_id, success);
    }
}

void AndroidParseWrapper::queryComplete ( int query_id, JNIEnv *env, const std::string& table, bool success )
{
    auto iter = _queries.find(query_id);
        for ( auto i : _delegates ) {
        	if (iter == _queries.end()) {
        		i->parseQueryComplete(query_id,false);
        		continue;
        	}
        	for ( auto j : iter->second ) {
    			std::shared_ptr<SaveClass> cpfobj = i->parseQueryCallback(query_id);
    			if (cpfobj) {
    				std::shared_ptr<ParseInterfaceObject> wrap (new ParseInterfaceObject (cpfobj, j));
    				wrap->loadObject();
    				i->parseQueryResult(wrap, query_id);
    			} else {

    			}
    		}
            i->parseQueryComplete(query_id,success);
        }
    _queries.erase(iter);
}



extern "C" {

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_queryComplete( JNIEnv* env, jclass clazz, jint query_id, jstring jtable, jboolean success)
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_queryComplete");
		std::string table = cocos2d::JniHelper::jstring2string(jtable);
		AndroidParseWrapper::get()->queryComplete ( query_id, env, table, success );
	}

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_queryStarted( JNIEnv* env, jclass clazz, jint query_id, jstring jtable, jboolean success)
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_queryComplete");
		std::string table = cocos2d::JniHelper::jstring2string(jtable);
		AndroidParseWrapper::get()->queryStarted ( query_id, table, success );
	}

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_parseLoginCallback( JNIEnv* env, jclass clazz, jint success, jboolean innew )
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_parseLoginCallback");
		AndroidParseWrapper::get()->parseLoginCallback( (ParseDelegate::LoginState)success, innew );
	}

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_objectLoaded( JNIEnv* env, jclass clazz, jint query_id, jobject obj, jstring jclass_name, jstring jobject_id, jboolean success )
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_objectLoaded");
		std::string class_name = cocos2d::JniHelper::jstring2string(jclass_name);
		std::string object_id = cocos2d::JniHelper::jstring2string(jobject_id);

		AndroidParseWrapper::get()->objectLoaded ( query_id, env, obj, success );
	}

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_queryLoaded( JNIEnv* env, jclass clazz, jint query_id, jobject obj )
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_queryLoaded");

		AndroidParseWrapper::get()->queryLoaded ( env, query_id, obj );
	}

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_queryObjectLoaded( JNIEnv* env, jclass clazz, jint query_id, jobject obj, jstring jclass_name, jstring jobject_id )
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_queryObjectLoaded");

		std::string class_name = cocos2d::JniHelper::jstring2string(jclass_name);
		std::string object_id = cocos2d::JniHelper::jstring2string(jobject_id);

		AndroidParseWrapper::get()->objectLoaded ( query_id, env, obj, true );
	}

	JNIEXPORT void JNICALL Java_com_bridge_parse_ParseBridge_objectSaved( JNIEnv* env, jclass clazz, jint query_id, jstring jclass_name, jstring jobject_id, jboolean success )
	{
		CCLOG("Java_com_bridge_parse_ParseBridge_objectSaved");

		std::string class_name = cocos2d::JniHelper::jstring2string(jclass_name);
		std::string object_id = cocos2d::JniHelper::jstring2string(jobject_id);

		AndroidParseWrapper::get()->objectSaved ( query_id, class_name, object_id, success );
	}
}
