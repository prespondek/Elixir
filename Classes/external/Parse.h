//
//  Parse.h
//  Elixir
//
//  Created by Peter Respondek on 9/15/15.
//
//

#ifndef __Elixir__Parse__
#define __Elixir__Parse__

#include "../cocos_extensions/Lanyard_Save.h"
#include "cocos2d.h"

class ParseWrapperObject
{
public:
	ParseWrapperObject( );
	virtual ~ParseWrapperObject();
    
    virtual std::string     getClassName        ( ) = 0;
    virtual std::string     getObjectId         ( ) = 0;
    virtual time_t          getUpdatedAt        ( ) = 0;
    virtual time_t          getCreatedAt        ( ) = 0;
    
	virtual void            getValue           ( const std::string& key, std::set<std::string>& value ) = 0;
    virtual void            getValue           ( const std::string& key, std::string& value ) = 0;
    virtual void            getValue           ( const std::string& key, unsigned int& value ) = 0;
    virtual void            getValue           ( const std::string& key, unsigned short& value ) = 0;
    virtual void            getValue           ( const std::string& key, long& value ) = 0;
    virtual void            getValue           ( const std::string& key, Date& value ) = 0;

	virtual void            setValue           ( const std::string& key, const std::set<std::string>& value ) = 0;
    virtual void            setValue           ( const std::string& key, const std::string& value ) = 0;
    virtual void            setValue           ( const std::string& key, const unsigned int& value ) = 0;
    virtual void            setValue           ( const std::string& key, const unsigned short& value ) = 0;
    virtual void            setValue           ( const std::string& key, const long& value ) = 0;
    virtual void            setValue           ( const std::string& key, const Date& value ) = 0;

};

class ParseInterfaceObject :  public SaveProtocol
{
public:
	ParseInterfaceObject( const std::shared_ptr<SaveClass>&, const std::shared_ptr<ParseWrapperObject>& );

	virtual void            loadObject           ( );
    virtual void            saveObject           ( );

    virtual void            loadValue           ( std::set<std::string>& 		value );
    virtual void            loadValue           ( std::string& 					value );
    virtual void            loadValue           ( unsigned int& 				value );
    virtual void            loadValue           ( unsigned short& 				value );
    virtual void            loadValue           ( long& 						value );
    virtual void            loadValue           ( Date& 						value );

    virtual void            saveValue           ( const std::set<std::string>& 	value );
    virtual void            saveValue           ( const std::string& 			value );
    virtual void            saveValue           ( const unsigned int& 			value );
    virtual void            saveValue           ( const unsigned short& 		value );
    virtual void            saveValue           ( const long& 					value );
    virtual void            saveValue           ( const Date& 					value );
    
    const std::shared_ptr<SaveClass>& getSaveObject ( ) { return _obj; }
    const std::shared_ptr<ParseWrapperObject>& getParseObject ( ) { return _jobj; }

protected:
	std::shared_ptr<SaveClass> _obj;
	std::shared_ptr<ParseWrapperObject> _jobj;
};

class ParseDelegate
{
    friend class ParseWrapper;
public:
    enum LoginState {
        SUCCESS,
        CANCELLED,
        BAD_CONNECTION,
        BAD_SERVER
    };
    virtual void            parseLoginCallback  ( LoginState success, bool new_user ) {}
    virtual void            parseLogOutCallback ( ) {}
    virtual void            parseSaveCallback   ( ) {}
    virtual void            parseLoadCallback   ( ) {}
    virtual void            parseObjectSaved    ( const std::shared_ptr<ParseInterfaceObject>& obj, int query_id, bool success ) {}
    virtual std::shared_ptr<SaveClass>
                            parseQueryCallback  ( int query_id ) { return nullptr; }
    virtual void            parseQueryResult    ( const std::shared_ptr<ParseInterfaceObject>& result, int query_id ) {}
    virtual void            parseQueryStarted   ( int query_id, bool success ) {}
    virtual void            parseQueryComplete  ( int query_id, bool success ) {}
};

class ParseWrapper : public cocos2d::Ref
{
public:
    
    static ParseWrapper* 	getInstance         ( );
    void                    purgeParse          ( );
    virtual void            loginWithFacebook   ( ) {}
    virtual void            logOut              ( ) {}

    virtual void            loadWithObjectId    ( const std::shared_ptr<SaveClass>&,
                                                  const std::string& key ) = 0;
    virtual int             saveWithObjectId    ( const std::shared_ptr<SaveClass>&,
                                                  const std::string& key ) = 0;
    virtual int             saveNewObject       ( const std::shared_ptr<SaveClass>& ) = 0;
    virtual void            deleteWithObjectId  ( const std::string& table,
                                                  const std::string& key ) = 0;
   
    virtual void            unpinWithName       ( const std::string& key ) = 0;
    
    // Queries
    virtual int             fetchUser           ( ) = 0;
    virtual int        		findObjectWithKey   ( const std::string& table,
                                                  const std::string& key,
                                                  const std::string& value,
                                                  bool fromDataStore = false ) = 0;
    virtual int        		getFriends          ( const std::vector<std::string>& friend_indices,
                                                  bool fromDataStore ) = 0;
    virtual int        		getMapScores        ( const std::string& obj_id,
                                                  uint16_t language,
                                                  uint16_t min,
                                                  uint16_t max,
                                                  bool fromDataStore ) = 0;
    
    virtual int             saveUser            ( const std::shared_ptr<SaveClass>&, bool cloud_save ) = 0;
    virtual std::shared_ptr<ParseInterfaceObject>
                            loadUser            ( const std::shared_ptr<SaveClass>& obj ) = 0;
    
    virtual bool            isLoggedIn          ( ) { return false; }
    
    void                    addDelegate         ( ParseDelegate* delegate );
    void                    removeDelegate      ( ParseDelegate* delegate );

    static ParseWrapper*    create              ( );
protected:
    virtual                 ~ParseWrapper       ( ) {}
                            ParseWrapper        ( );
    int                		incrementQueryId    ( );


    virtual bool            init                ( );
    
    std::vector<ParseDelegate*>                         _delegates;
    int                                                 _query_id;
};


#endif
