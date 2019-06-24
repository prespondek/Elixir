//
//  Parse.cpp
//  Elixir
//
//  Created by Peter Respondek on 9/15/15.
//
//

#include "Parse.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "../../proj.ios_mac/ios/IOSParse.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../../proj.android/app/jni/AndroidParse.h"

#endif

static ParseWrapper* s_Parse = nullptr;

ParseInterfaceObject::ParseInterfaceObject( const std::shared_ptr<SaveClass>& obj,
											const std::shared_ptr<ParseWrapperObject>& jobj) :
_jobj(jobj),
_obj(obj)
{ }

void ParseInterfaceObject::loadObject ( )
{
    SaveProtocol::loadObject(_obj.get());
}

void ParseInterfaceObject::saveObject ( )
{
    SaveProtocol::saveObject(_obj.get());
}

void ParseInterfaceObject::loadValue ( std::set<std::string>& value )
{
    CCLOG("ParseWrapperObject::loadValue_ss");
    for ( auto& v : value ) {
        CCLOG("%s", v.c_str());
    }
	_jobj->getValue(_key,value);
}

void ParseInterfaceObject::loadValue ( std::string& value )
{
    CCLOG("ParseWrapperObject::loadValue_str %s : %s", _key.c_str(), value.c_str());
	_jobj->getValue(_key,value);
}

void ParseInterfaceObject::loadValue ( unsigned int& value )
{
    CCLOG("ParseWrapperObject::loadValue_i %s : %u", _key.c_str(), value);
	_jobj->getValue(_key,value);
}

void ParseInterfaceObject::loadValue ( unsigned short& value )
{
    CCLOG("ParseWrapperObject::loadValue_s %s : %hu", _key.c_str(), value);
	_jobj->getValue(_key,value);
}

void ParseInterfaceObject::loadValue ( long& value )
{
    CCLOG("ParseWrapperObject::loadValue_l %s : %ld", _key.c_str(), value);
	_jobj->getValue(_key,value);

}

void ParseInterfaceObject::loadValue ( Date& value )
{
    CCLOG("ParseWrapperObject::loadValue_d %s : %ld", _key.c_str(), value.getTime());
	_jobj->getValue(_key,value);
}

void ParseInterfaceObject::saveValue ( const std::set<std::string>& value )
{
	CCLOG("ParseWrapperObject::saveValue_ss");
    for ( auto& v : value ) {
        CCLOG("%s", v.c_str());
    }
	_jobj->setValue(_key,value);
}

void ParseInterfaceObject::saveValue ( const std::string& value )
{
	CCLOG("ParseWrapperObject::saveValue_str %s : %s", _key.c_str(), value.c_str());
	_jobj->setValue(_key,value);
}

void ParseInterfaceObject::saveValue ( const unsigned int& value )
{
    CCLOG("ParseWrapperObject::saveValue_i %s : %u", _key.c_str(), value);
	_jobj->setValue(_key,value);
}

void ParseInterfaceObject::saveValue ( const unsigned short& value )
{
    CCLOG("ParseWrapperObject::saveValue_s %s : %hu", _key.c_str(), value);
	_jobj->setValue(_key,value);
}

void ParseInterfaceObject::saveValue ( const long& value )
{
    CCLOG("ParseWrapperObject::saveValue_l %s : %ld", _key.c_str(), value);
	_jobj->setValue(_key,value);
}

void ParseInterfaceObject::saveValue ( const Date& value )
{
	CCLOG("ParseWrapperObject::saveValue_d %s : %ld", _key.c_str(), value.getTime());
	_jobj->setValue(_key,value);
}

ParseWrapper::ParseWrapper() :
_query_id(0)
{

}

ParseWrapper* ParseWrapper::getInstance()
{
    if (!s_Parse) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    	s_Parse = new IOSParseWrapper();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    	s_Parse = new AndroidParseWrapper();
#endif
    	s_Parse->init();
    }
    return s_Parse;
}

void ParseWrapper::purgeParse()
{
    CC_SAFE_DELETE(s_Parse);
    s_Parse = NULL;
}

bool ParseWrapper::init()
{
    return true;
}

ParseWrapperObject::ParseWrapperObject( )
{
}

ParseWrapperObject::~ParseWrapperObject()
{

}

void ParseWrapper::addDelegate(ParseDelegate* delegate)
{
	CCLOG("Parse delegate added %p", delegate);
	_delegates.push_back(delegate);
}

void ParseWrapper::removeDelegate(ParseDelegate* delegate)
{
	CCLOG("Parse delegate removed %p", delegate);
	auto i = std::find (_delegates.begin(), _delegates.end(), delegate);
	if (i != _delegates.end()) {
		_delegates.erase(i);
	}
}

int ParseWrapper::incrementQueryId()
{
    _query_id++;
    if (_query_id == 0) _query_id++;
    return _query_id;
}
