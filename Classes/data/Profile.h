//
//  Player.h
//  Pirateer
//
//  Created by Peter Respondek on 1/24/14.
//
//

#ifndef __Elixir_Profile__
#define __Elixir_Profile__

struct FacebookGameRequest;

#include "cocos2d.h"
#include "ElixirTypes.h"
#include "../cocos_extensions/Lanyard_Save.h"
#include "../external/Parse.h"
#include <set>

USING_NS_CC;

using namespace Lanyard;

const int REGEN_TIME = 1800;

struct DailyStats
{
    time_t      m_day = 0;
    float       m_distance = 0.0f;
    uint32_t    m_money = 0;
    uint32_t    m_voyages = 0;
};

class PlayerStats : public Ref
{
public:
    PlayerStats();
    virtual ~PlayerStats();
    
    CREATE_FUNC(PlayerStats);
    
    bool        init();

};

class PlayerPrefs : public Ref
{
public:
    PlayerPrefs();
    
    CREATE_FUNC(PlayerPrefs);
    bool init();
    
    virtual ~PlayerPrefs();
    
    void updatePrefs();
    float getVolume() const;
    
private:
    
    float m_volume;
};

class ProfileDelegate
{
    friend class Profile;
private:
    virtual void regenTimeRemaining                     ( time_t time ) {}
    virtual void lifeRestored                           ( ) {}
    virtual void ambrosiaChanged                        ( uint16_t old_value ) {}
    virtual void avatarTextureCached                    ( ) {}
    virtual void friendQueryComplete                    ( bool success ) {}
    virtual void requestsUpdated                        ( ) {}
};

class Profile : public Ref, public SaveProtocol, public ParseDelegate
{
    friend class ProfileDelegate;

public:
    enum DebugOptions {
        INFINITE_LIVES =    0x01,
        INFINITE_MANA =     0x02,
        INFINITE_MOVES =    0x04,
        ALL_LEVELS =        0x08,
        TABLE_BOT =         0x10,
        CLEAR_STATS =       0x20,
        CONTINUOUS_PLAY =   0x40,
        ROTATE_LEVELS =     0x80,
        SPEED_UP =          0x100,
        SHOW_STATS =        0x200,
        TRACK_TOUCH =       0x400
    };
    
    struct FacebookFriendData {
        std::string picture;
        bool permission;
    };
    
    Profile                                             ( );
    virtual ~Profile                                    ( );
    
    CREATE_FUNC(Profile);
    static std::shared_ptr<SaveClass> createUserData    ( );
    static std::shared_ptr<SaveClass> createProfileQuery( );
    
    bool                getDebugFlag                    ( DebugOptions state );
    void                setDebugFlag                    ( DebugOptions state, bool val );
    void                setDebugPlays                   ( uint16_t plays );
    uint8_t             fetchDebugPlays                 ( );
    uint8_t             getDebugPlays                   ( );
    
    void                addLife                         ( );
    void                addExtraLife                    ( );
    void                copyProfile                     ( Profile* profile );
    void                restoreLife                     ( );
    void                addDelegate                     ( ProfileDelegate* delegate );
    void                removeDelegate                  ( ProfileDelegate* delegate );
    uint16_t            getCurrentLevel                 ( );
    void                setAvatarIdx                    ( uint16_t idx );
    uint16_t            getAvatarIdx                    ( );
    void                setCurrentLevel                 ( uint16_t lvl );
    uint32_t            getLevelScore                   ( uint16_t lvl );
    void                setLevelScore                   ( uint16_t lvl, uint32_t score );
    void                setUserName                     ( const std::string& name );
    void                setUserId                       ( const std::string& str );
    void                unlockLevel                     ( );
    void                setEmail                        ( const std::string& str );
    void                setAmbrosia                     ( uint16_t amb );
    void                setFBID                         ( const std::string& str );
    void                setPurchase                     ( const std::string& str );
    void                setUUID                         ( const std::string& str );
    void                addParseFriend                  ( const std::string&str );
    void                addFacebookFriend               ( const std::pair<std::string,FacebookFriendData> &str );
    void                clearFacebookFriends            ( );
    void                clearFriendData                 ( );
    void                addFriendData                   ( const std::shared_ptr<ParseInterfaceObject>& data );
    void                removeLife                      ( );
    time_t              getParseQueryTime               ( );
    const std::string&  getUserName                     ( );
    const std::string&  getUUID                         ( );
    const std::string&  getFBID                         ( );
    const std::string&  getUserId                       ( );
    uint16_t            getUnlockedLevel                ( );
    uint16_t            getLives                        ( );
    uint16_t            getLanguage                     ( );
    uint16_t            getMaxLives                     ( );
    uint16_t            getExtraLives                   ( );
    uint16_t            getRegenLevel                   ( );
    uint16_t            getMaxLevel                     ( );
    time_t              getGameTime                     ( );
    uint16_t            getAmbrosia                     ( );
    uint16_t            getRegenTime                    ( );
    void                addRegenLevel                   ( );
    const std::shared_ptr<SaveClass>& getUserData       ( );
    std::shared_ptr<SaveClass>
                        getFriendDataWithUUID           ( const std::string& user_id );
    void                queryFriendData                 ( );
    
    void                addGameRequestData              ( const std::shared_ptr<FacebookGameRequest>& req);
    void                clearGameRequestData            ( );
    void                addGameRequestId                ( const std::string& reqs );
    bool                isPendingGameRequests           ( );
    void                clearGameRequestId              ( );
    void                getPendingRequest               ( std::shared_ptr<FacebookGameRequest>& req );
    void                removeGameRequest               ( const std::shared_ptr<FacebookGameRequest>& req );
    size_t              getNumGameRequests              ( );
    const std::vector<std::shared_ptr<FacebookGameRequest>>*
                        getFriendRequests               ( const std::string& uuid );
    void transitionProfile                          ( Profile* other );
    void                postLocalNotifications          ( );


    virtual std::shared_ptr<SaveClass>
                            parseQueryCallback          ( int query_id );
    virtual void            parseQueryResult            ( const std::shared_ptr<ParseInterfaceObject>& result, int query_id );
    virtual void            parseQueryStarted           ( int query_id, bool success );
    virtual void            parseQueryComplete          ( int query_id, bool success );
    
    const std::set<std::string>& getFriendIds           ( );
    
    void                setUserData                     ( const std::shared_ptr<SaveClass>& profile );
    std::shared_ptr<SaveClass> copyUserData             ( );
    
    void                save                            ( );
    void                load                            ( );
    
    void                update                          ( float dt );
    
    virtual void        loadValue                       ( std::set<std::string>& value );
    virtual void        loadValue                       ( std::string& value );
    virtual void        loadValue                       ( unsigned int& value );
    virtual void        loadValue                       ( unsigned short& value );
    virtual void        loadValue                       ( long& value );
    virtual void        loadValue                       ( Date& date );
    
    virtual void        saveValue                       ( const std::set<std::string>& value );
    virtual void        saveValue                       ( const std::string& value );
    virtual void        saveValue                       ( const unsigned int& value );
    virtual void        saveValue                       ( const unsigned short& value );
    virtual void        saveValue                       ( const long& value );
    virtual void        saveValue                       ( const Date& date );
    
    const std::string&  getAvatarUrl                    ( );
    void                setAvatarUrl                    ( const std::string& url );
    
    CC_SYNTHESIZE(bool,                     _notification,                      Notifications);
    CC_SYNTHESIZE(bool,                     _fb_permission,                     FBPermission);
    CC_SYNTHESIZE(time_t,                   _time_stamp_session,                SessionTime);
    CC_SYNTHESIZE_PASS_BY_REF(std::string,  _test_level,                        TestLevel);
    CC_SYNTHESIZE(bool,                     _new_user,                          NewUser);
    CC_SYNTHESIZE_READONLY_PASS_BY_REF(std::vector<std::shared_ptr<ParseInterfaceObject>>, _friend_data,   FriendData);
    //CC_SYNTHESIZE_READONLY(bool,            _friend_query_complete,             FriendQueryComplete);
    
protected:
    bool init                                       ( );

private:
    
    time_t                              _fb_query_time;
    std::map<std::string,FacebookFriendData> _fb_friends;
    time_t                              _parse_query_time;
    
    std::shared_ptr<SaveClass>          _user_data;
    uint16_t                            _friend_query_id;
    
    std::map<std::string,std::vector<std::shared_ptr<FacebookGameRequest>>>
                                        _game_reqs;
    SaveValue<std::set<std::string>>*   _parse_friends;
    SaveValue<std::string>*             _user_id;
    SaveValue<std::string>*             _fb_id;
    SaveList<uint32_t>*                 _level_scores;
    SaveValue<uint16_t>*                _level;
    SaveValue<uint16_t>*                _avatar_idx;
    SaveValue<uint16_t>*                _max_level;
    SaveValue<uint16_t>*                _ambrosia;
    SaveValue<uint16_t>*                _language;
    SaveList<time_t>*                   _lives;
    SaveValue<uint16_t>*                _max_lives;
    SaveValue<uint16_t>*                _extra_lives;
    SaveValue<uint16_t>*                _regen_level;                // time to regen a life
    SaveValue<std::string>*             _user_name;
    SaveValue<std::string>*             _avatar_url;
    SaveValue<std::string>*             _uuid;
    SaveValue<std::string>*             _email;
    SaveValue<std::string>*             _pending_purchase;
    SaveValue<time_t>*                  _time_stamp_total;          // time since game was first launched
    SaveValue<Date>*                    _time_stamp_save;
    
    std::vector<ProfileDelegate*>       _delegates;
    uint16_t                            _debug_flags;

};

inline void                 Profile::clearGameRequestData( ) { _game_reqs.clear(); }

inline void                 Profile::clearFriendData      ( ) { _friend_data.clear(); }
inline void                 Profile::addFriendData      ( const std::shared_ptr<ParseInterfaceObject>& data ) { _friend_data.push_back(data); }

inline time_t               Profile::getParseQueryTime ( ) { return difftime(time(NULL), _parse_query_time); }

inline void                 Profile::addParseFriend    ( const std::string& str )
{ _parse_friends->getValue().insert(str); }

inline void                 Profile::clearFacebookFriends ( ) { _fb_friends.clear(); }
inline void                 Profile::addFacebookFriend ( const std::pair<std::string,FacebookFriendData> &str ) { _fb_friends.insert(str); }

inline const std::shared_ptr<SaveClass>&  Profile::getUserData        ( ) { return _user_data; }

inline void                 Profile::setFBID            ( const std::string &str ) { _fb_id->setValue(str); }

inline void                 Profile::setUUID            ( const std::string &str ) { _uuid->setValue(str); }

inline void                 Profile::setEmail           ( const std::string &str ) { _email->setValue(str); }

inline const std::string&   Profile::getUserId          ( ) { return _user_id->getValue(); }

inline void                 Profile::setUserId          ( const std::string& str ) { return _user_id->setValue(str); }

inline const std::string&   Profile::getAvatarUrl       ( ) { return _avatar_url->getValue(); }

inline const std::set<std::string>& Profile::getFriendIds( ) { return _parse_friends->getValue(); }

inline void                 Profile::setAvatarUrl       ( const std::string& str ) { _avatar_url->setValue(str); }

inline const std::string&   Profile::getUUID            ( ) { return _uuid->getValue(); }

inline const std::string&   Profile::getFBID            ( ) { return _fb_id->getValue(); }

inline const std::string&   Profile::getUserName        ( ) { return _user_name->getValue(); }

inline uint16_t             Profile::getCurrentLevel    ( ) { return _level->getValue(); }

inline void                 Profile::setCurrentLevel    ( uint16_t lvl ) { _level->setValue(lvl); }

inline uint16_t             Profile::getUnlockedLevel   ( ) { return getMaxLevel(); }

inline uint16_t             Profile::getLives           ( ) { return _max_lives->getValue() -
                                                                     _lives->getValues().size(); }

inline uint16_t             Profile::getExtraLives      ( ) { return _extra_lives->getValue(); }

inline uint16_t             Profile::getMaxLives        ( ) { return _max_lives->getValue(); }

inline uint16_t             Profile::getLanguage        ( ) { return _language->getValue(); }

inline uint16_t             Profile::getAmbrosia        ( ) { return _ambrosia->getValue(); }


inline void                 Profile::setAvatarIdx       ( uint16_t idx ) { return _avatar_idx->setValue(idx); }

inline uint16_t             Profile::getMaxLevel        ( ) { if ( getDebugFlag(ALL_LEVELS) ) return 9999;
                                                              return _max_level->getValue(); }

inline time_t               Profile::getGameTime        ( ) { return _time_stamp_total->getValue(); }


inline void             Profile::addRegenLevel       ( ) { _regen_level->setValue(_regen_level->getValue() + 1); }

inline uint16_t             Profile::getRegenLevel       ( ) { return _regen_level->getValue(); }

inline bool                 Profile::getDebugFlag       ( DebugOptions state ) { return _debug_flags & state; }

inline void                 Profile::setDebugFlag       ( DebugOptions state, bool val ) {
    if (val) _debug_flags |= state;
    else _debug_flags &= ~state;
    UserDefault::getInstance()->setIntegerForKey("DebugFlags", _debug_flags);
}

#endif /* defined(__Pirateer__Player__) */
