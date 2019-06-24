//
//  IOSParse.h
//  Elixir
//
//  Created by Peter Respondek on 9/16/15.
//
//

#ifndef __Elixir__IOSParseWrapper__
#define __Elixir__IOSParseWrapper__


#include "../Classes/external/Parse.h"

class IOSParseWrapperObject : public ParseWrapperObject
{
public:
    IOSParseWrapperObject( void* pobj );
    virtual ~IOSParseWrapperObject();
    
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
    
    void                    setPObject          ( void* _pobj );
    
    void* _pobj;
};

class IOSParseWrapper : public ParseWrapper {
    friend class ParseWrapper;
    
    virtual void            loginWithFacebook ();
    virtual void            logOut ();
    
    /*virtual void            loadWithQuery       ( SaveObject* obj,
                                                  const std::string& key,
                                                  const std::string& value );*/
    virtual void            loadWithObjectId    ( const std::shared_ptr<SaveClass>& obj,
                                                  const std::string& key );
    virtual int             saveWithObjectId    ( const std::shared_ptr<SaveClass>& obj,
                                                  const std::string& key );
    virtual void            deleteWithObjectId  ( const std::string& table,
                                                  const std::string& key );
    virtual int             saveNewObject       ( const std::shared_ptr<SaveClass>& obj);

    virtual void            unpinWithName       ( const std::string& key );
    
    virtual int             getFriends          ( const std::vector<std::string>& friend_indices,
                                                  bool fromDatastore );
    virtual int             getMapScores        ( const std::string& obj_id,
                                                  uint16_t language,
                                                  uint16_t min,
                                                  uint16_t max,
                                                  bool fromDatastore );
    virtual int             findObjectWithKey   ( const std::string& table,
                                                  const std::string& key,
                                                  const std::string& value,
                                                  bool fromDataStore = false );
    
    virtual int             saveUser ( const std::shared_ptr<SaveClass>& obj, bool cloud_save );
    virtual std::shared_ptr<ParseInterfaceObject>
                            loadUser ( const std::shared_ptr<SaveClass>& obj);
    
    virtual bool            isLoggedIn();
    
    virtual int             fetchUser           ( );



private:

    IOSParseWrapper();
    
    void                    queryCallback       ( void* result, void* error, const std::string& table_name, int query_id );
};

#endif




