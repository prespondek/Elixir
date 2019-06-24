//
//  Game.h
//  Elixir
//
//  Created by Peter Respondek on 9/15/15.
//
//

#ifndef __Elixir__Game__
#define __Elixir__Game__

class ElixirScene;

#include "../data/Profile.h"
#include "../data/DownloadManager.h"
#include "../data/TableData.h"
#include "../external/Parse.h"
#include "cocos2d.h"
#include "../external/Facebook.h"
#include "../external/Reachability.h"

#define FONTSCALE(__STRING__) \
Game::getInstance()->getImageScale() * __STRING__ \

USING_NS_CC;

enum LoginStage {
    PARSE_STAGE             = 0x01,
    REQUEST_STAGE           = 0x02,
    FACEBOOK_FRIEND_STAGE   = 0x04,
    USER_STAGE              = 0x08,
    PORTRAIT_STAGE          = 0x10,
    PARSE_FRIEND_STAGE      = 0x20
};

class GameDelegate
{
public:
    enum GameInteruptType {
        NONE,
        LOGIN,
        FACEBOOK,
        REACHABILITY,
        FOREGROUND
    };
    // called once parse facebook and parse logs in and profile image is downloaded.
    virtual void gameLogIn              ( bool new_user )                   { }
    // called once parse and facebook log out but before cache is cleared.
    virtual void gameLogOut             ( Profile* profile )                { }
    virtual void mapQueryComplete       ( bool success )                    { }
    virtual void portraitFinished       ( Sprite* sprite )                  { }
    virtual void gameRequestsComplete   ( bool success )                    { }
    virtual void gameLoginStage         ( LoginStage stage )                { }
    virtual void saveMismatch           ( const std::shared_ptr<SaveClass>& cloud,
                                          const std::shared_ptr<SaveClass>& local )  { }
    virtual void userSaved              ( bool success )                    { }
    virtual void textureDownloaded      ( Texture2D* tex )                  { }
    
    virtual void recieveInterupt        ( GameInteruptType interupt )       { }
    virtual void clearInterupt          ( GameInteruptType interupt, bool success, const std::string& error ) {}

};

class Game : public Ref,
             public FacebookDelegate,
             public ParseDelegate,
             public ReachabilityDelegate,
             public ProfileDelegate
{
public:
    
    virtual ~Game();
    
    static Game*    		getInstance     ( );
    
    //static GameRequestType getRequestType   ( FacebookGameRequest* req );
    //static std::string getRequestString     ( GameRequestType req );
    
    Profile*         getProfile             ( );
    DownloadManager* getDLManager           ( );
    TableData*       getTableData           ( );
    Texture2D*       downloadTexture        ( std::string& url,
                                              std::string& file );
    
    bool            checkLoginStage         ( LoginStage state );
    bool            isInitalised            ( );
    uint16_t        getMaxMapIndex          ( );
    uint16_t        getMaxLevel             ( );
    uint8_t         getCurrentLevelMap      ( );
    uint8_t         findMapIdxWithLevelIdx  ( uint16_t level );
    bool            getAvatarTexture        ( Texture2D*& tex );
    std::string     getAvatarPath           ( );
    Vec2            getLevelRangeForMap     ( uint8_t map_idx );
    
    Sprite*         downloadCharacterPortrait ( SaveClass* data );
    bool            isIncomingURL           ( std::string& url,
                                              std::string& source );
    
    void            deleteGameRequest       ( const std::shared_ptr<FacebookGameRequest>& reqs );
    //void            consumeGameRequest      ( const std::shared_ptr<FacebookGameRequest>& reqs );
    void            appInviteComplete       ( bool success );

    void            queryMapScores          ( );
    void            queryFriendData         ( );
    
    void            parseObjectSaved        ( const std::shared_ptr<ParseInterfaceObject>& obj,
                                              int query_id, bool success );
    void            parseLoginCallback      ( LoginState success, bool new_user );
    void            parseLogOutCallback     ( );
    std::shared_ptr<SaveClass>
                    parseQueryCallback      ( int query_id );
    void            parseQueryStarted       ( int query_id, bool success );
    void            parseQueryComplete      ( int query_id, bool success );
    void            parseQueryResult        ( const std::shared_ptr<ParseInterfaceObject>& obj, int query_id );
    void            reset                   ( );

    bool            isNetworkReachable      ( );
    bool            isLoggedIn              ( );
    void            didShowInterupt         ( GameDelegate::GameInteruptType interupt );

    
    void            addDelegate             ( GameDelegate* delegate );
    void            removeDelegate          ( GameDelegate* delegate );
        
    void            save                    ( bool cloud_save, bool cloud_check );
    void            load                    ( );
    void            fetchFriendData         ( );

    bool            isTestMode              ( );
    void            setTestMode             ( bool flag );
    void            setProfile              ( Profile* profile );
    void            clearCloudProfile       ( );
    void            sendInterupt            ( );
    float           getImageScale           ( );
    void            setImageScale           ( float scale );
    CC_PROPERTY                             ( uint8_t, _map_idx, MapIndex );
    CC_SYNTHESIZE                           ( uint8_t, _prev_map_idx, PrevMapIndex );
    CC_SYNTHESIZE_READONLY_PASS_BY_REF      ( std::vector<std::shared_ptr<ParseInterfaceObject>>, _query_data, QueryData );
    CC_SYNTHESIZE_READONLY                  ( bool, _query_complete, QueryComplete );

protected:
    Game();
    bool init ();
    
    void            incrementLoginStage     ( LoginStage stage );
    void            reachabilityStatusChanged( ReachabilityStatus status );

    void            dlUserPortrait          ( );

    void            loginFinished           ( );
    void            errorCallback           ( long error );
    void            userLoggedIn            ( bool success );
    void            userDataCallback        ( const std::map<std::string,std::string>& me_data );
    void            friendDataCallback      ( const std::vector<std::map<std::string,std::string>>& friends_data );
    void            friendQueryComplete     ( bool success );
    void            textureLoadedCallback   ( Texture2D* tex );
    void            gameRequestRecievedCallback ( const std::set<std::string>& reqs );
    void            gameRequestDeletedCallback ( bool success, const std::shared_ptr<FacebookGameRequest>& req );
    void            gameRequestsCallback    ( bool success,
                                             const std::map<std::string, std::shared_ptr<FacebookGameRequest>>& req);


    int                                                     _query_id;
    int                                                     _profile_id;
    int                                                     _user_id;
    std::vector<std::shared_ptr<ParseInterfaceObject>>      _profile_results;
    float                                                   _image_scale;
    bool                                                    _test_mode;
    bool                                                    _net_reachable;
    uint8_t                                                 _state;
    bool                                                    _login;
    uint16_t                                                _max_level;
    std::vector<uint16_t>                                   _map_ranges;
    std::vector<GameDelegate*>                              _delegates;
    Profile*                                                _local_profile;
    std::shared_ptr<ParseInterfaceObject>                   _cloud_profile;
    DownloadManager*                                        _dlmanager;
    TableData*                                              _table_data;
    bool													_pending_interupt;
};

inline bool             Game::checkLoginStage( LoginStage state )       { return _state & state; }
inline uint16_t         Game::getMaxLevel()         { return _max_level; }
inline uint8_t          Game::getMapIndex() const   { return _map_idx; }
inline bool             Game::isNetworkReachable()  { return _net_reachable; }
inline Profile*         Game::getProfile()          { return _local_profile; }
inline DownloadManager* Game::getDLManager()        { return _dlmanager; }
inline TableData*       Game::getTableData()        { return _table_data; }
inline bool             Game::isTestMode()          { return _test_mode; }
inline void             Game::setTestMode( bool flag )         { _test_mode = flag; }
inline float            Game::getImageScale()       { return _image_scale; }
inline void             Game::setImageScale( float scale )       { _image_scale = scale; }

#endif /* defined(__Elixir__Game__) */
