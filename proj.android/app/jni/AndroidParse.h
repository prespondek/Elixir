#ifndef ANDROID_PARSE_H
#define ANDROID_PARSE_H

#include "external/Parse.h"
#include <functional>
#include <map>
#include <jni.h>

class AndroidParseWrapperObject : public ParseWrapperObject
{
public:
	AndroidParseWrapperObject( jobject jobj, JNIEnv* env  );
	virtual ~AndroidParseWrapperObject( );

	static AndroidParseWrapperObject*	get		( ParseWrapperObject* obj );
	virtual std::string     getClassName        ( );
	virtual std::string     getObjectId         ( );
	virtual time_t          getUpdatedAt        ( );
	virtual time_t          getCreatedAt        ( );

	virtual void            getValue           ( const std::string& key, std::set<std::string>& value );
	virtual void            getValue           ( const std::string& key, std::string& value );
	virtual void            getValue           ( const std::string& key, unsigned int& value );
	virtual void            getValue           ( const std::string& key, unsigned short& value );
	virtual void            getValue           ( const std::string& key, long& value );
	virtual void            getValue           ( const std::string& key, Date& value );

	virtual void            setValue           ( const std::string& key, const std::set<std::string>& value );
	virtual void            setValue           ( const std::string& key, const std::string& value );
	virtual void            setValue           ( const std::string& key, const unsigned int& value );
	virtual void            setValue           ( const std::string& key, const unsigned short& value );
	virtual void            setValue           ( const std::string& key, const long& value );
	virtual void            setValue           ( const std::string& key, const Date& value );

    void 					setJObject 			( jobject jobj, JNIEnv* env = nullptr );
    void 					clearJObject 		( JNIEnv* env = nullptr );

    jobject					getJObject			( );

protected:

    jobject _jobj;
    JNIEnv* _env;
};

class AndroidParseWrapper : public ParseWrapper {
public:
    friend class ParseWrapper;
    friend class AndroidParseWrapperObject;

    						AndroidParseWrapper	( );
    virtual                 ~AndroidParseWrapper( );

    virtual void            loginWithFacebook 	( );
    virtual void            logOut 				( );

    static AndroidParseWrapper* get				( );

    virtual void            loadWithObjectId    ( const std::shared_ptr<SaveClass>&,
                                                  const std::string& key );
    virtual int             saveWithObjectId    ( const std::shared_ptr<SaveClass>&,
                                                  const std::string& key );
    virtual int             saveNewObject       ( const std::shared_ptr<SaveClass>& );
    virtual void            deleteWithObjectId  ( const std::string& table,
                                                  const std::string& key );

    virtual void            unpinWithName       ( const std::string& key );

    // Queries
    virtual int             fetchUser           ( );
    virtual int        		findObjectWithKey   ( const std::string& table,
                                                  const std::string& key,
                                                  const std::string& value,
                                                  bool fromDataStore = false );
    virtual int        		getFriends          ( const std::vector<std::string>& friend_indices,
                                                  bool fromDataStore );
    virtual int        		getMapScores        ( const std::string& obj_id,
													 uint16_t language,
													 uint16_t min,
                                                  uint16_t max,
                                                  bool fromDataStore );

    virtual int             saveUser            ( const std::shared_ptr<SaveClass>&, bool cloud_save );
    virtual std::shared_ptr<ParseInterfaceObject>
                            loadUser            ( const std::shared_ptr<SaveClass>& obj );

    virtual bool            isLoggedIn			( );

    void            		parseLoginCallback 	( ParseDelegate::LoginState login, bool is_new );
    void 					objectSaved 		( int query_id, const std::string& name, const std::string& object_id, bool success );
    void 					objectLoaded 		( int query_id, JNIEnv* env, jobject obj, bool success );

    void 					queryLoaded 		( JNIEnv* env, int query_id, jobject jobj );
	void 					queryComplete 		( int query_id, JNIEnv* env, const std::string& table, bool success );
	void 					queryStarted 		( int query_id, const std::string& table, bool success );

private:
    jobject					createObject 		( JniMethodInfo& t, std::string class_name, std::string obj_id );

    std::map<int,std::shared_ptr<ParseInterfaceObject>>::iterator
    						makeSaveWrapper 	( const std::shared_ptr<SaveClass>& obj, jobject jobj, JNIEnv* env );

    int 					createQuery 		( );


    std::map<int,std::shared_ptr<ParseInterfaceObject>> 	_reqs;
    std::map<int,std::vector<std::shared_ptr<AndroidParseWrapperObject>>> _queries;

    jmethodID _class_name;
    jmethodID _object_id;
    jmethodID _updated_at;
    jmethodID _created_at;

public:
    jmethodID _jhas_value;
    jmethodID _jload_value_ss;
    jmethodID _jload_value_str;
    jmethodID _jload_value_s;
    jmethodID _jload_value_i;
    jmethodID _jload_value_l;
    jmethodID _jload_value_d;

    jmethodID _jsave_value_ss;
    jmethodID _jsave_value_str;
    jmethodID _jsave_value_s;
    jmethodID _jsave_value_i;
    jmethodID _jsave_value_l;
    jmethodID _jsave_value_d;

    jclass _jparse_bridge;
};

inline jobject AndroidParseWrapperObject::getJObject () { return _jobj; }

#endif
