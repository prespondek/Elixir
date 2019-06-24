//
//  Game.cpp
//  Elixir
//
//  Created by Peter Respondek on 9/15/15.
//
//

#include "../data/Game.h"
#include "../external/Parse.h"
#include "../nodes/ElixirNodes.h"
#include "../scenes/ElixirScene.h"
#include "../data/ElixirLocalization.h"
#include "../external/Fabric.h"


static Game* g_Game = nullptr;

Game* Game::getInstance()
{
    if (g_Game == nullptr) {
        g_Game = new (std::nothrow) Game();
        if (!g_Game || !g_Game->init()) {
            CC_SAFE_DELETE(g_Game);
            return nullptr;
        }
    }
    return g_Game;
}


Game::~Game()
{
    ParseWrapper::getInstance()->removeDelegate(this);
    _local_profile->removeDelegate(this);
    
    CC_SAFE_RELEASE(_local_profile);
    CC_SAFE_RELEASE(_dlmanager);
    CC_SAFE_RELEASE(_table_data);
    ParseWrapper::getInstance()->purgeParse();
    
    Facebook::getInstance()->removeDelegate(this);
    Reachability::getInstance()->removeDelegate(this);
}

Game::Game() :
_local_profile      (nullptr),
_cloud_profile      (nullptr),
_table_data         (nullptr),
_dlmanager          (nullptr),
_test_mode          (false),
_state              (0),
_login              (false),
_net_reachable      (false),
_query_id           (0),
_map_idx            (0),
_prev_map_idx       (0),
_profile_id         (0),
_query_complete     (false),
_user_id            (0),
_pending_interupt   (NONE),
_max_level          (0),
_image_scale        (1.0f)
{
    
}

void Game::setMapIndex(uint8_t var)
{
    _prev_map_idx = _map_idx;
    _map_idx = var;
}

void Game::incrementLoginStage( LoginStage stage )
{
    if (!(_state & stage)) {
        switch (stage) {
            case PORTRAIT_STAGE: {
                CCLOG("incrementLoginStage:PORTRAIT_STAGE");
            }
                break;
            case FACEBOOK_FRIEND_STAGE: {
                CCLOG("incrementLoginStage:FACEBOOK_FRIEND_STAGE");
                queryFriendData();
            }
                break;
            case PARSE_FRIEND_STAGE: {
                CCLOG("incrementLoginStage:PARSE_FRIEND_STAGE");
            }
                break;
            case USER_STAGE: {
                CCLOG("incrementLoginStage:USER_STAGE");
                dlUserPortrait();
            }
                break;
            case REQUEST_STAGE: {
                CCLOG("incrementLoginStage:REQUEST_STAGE");
            }
                break;
            default:
                break;
        }
    }
    uint8_t login = _state | stage;
    if (login != _state) {
        _state = login;
        for ( auto i : _delegates ) {
            i->gameLoginStage(stage);
        }
        if (_login == false && isInitalised() ) {
            _login = true;
            for ( auto i : _delegates ) {
                i->gameLogIn(_local_profile->getNewUser());
            }
        }
    }
}

bool Game::isLoggedIn( )
{
    return _login;
}

bool Game::isInitalised( )
{
    return _state == uint8_t( PORTRAIT_STAGE | FACEBOOK_FRIEND_STAGE | PARSE_FRIEND_STAGE | PARSE_STAGE | USER_STAGE | REQUEST_STAGE );
}


void Game::addDelegate(GameDelegate *delegate)
{
    auto i = std::find(_delegates.begin(), _delegates.end(), delegate);
    if (i == _delegates.end()) {
        _delegates.push_back(delegate);
    }
}

void Game::removeDelegate ( GameDelegate *delegate )
{
    auto i = std::find(_delegates.begin(), _delegates.end(), delegate);
    if (i != _delegates.end()) {
        _delegates.erase(i);
    }
}

void Game::loginFinished ( )
{
    Facebook::getInstance()->getUserData();
    fetchFriendData();
}

void Game::fetchFriendData( )
{
    _state &= ~REQUEST_STAGE;
    _state &= ~FACEBOOK_FRIEND_STAGE;
    _state &= ~PARSE_FRIEND_STAGE;
    Facebook::getInstance()->getFriendData();
    Facebook::getInstance()->getGameRequests(facebook_app_id);

}

std::string Game::getAvatarPath ( )
{
    return _dlmanager->getCacheLocation() + "avatar/" + _local_profile->getFBID() + ".jpg";
}

bool Game::getAvatarTexture ( Texture2D*& tex )
{
    std::string avatar_location = getAvatarPath();
    tex = Director::getInstance()->getTextureCache()->addImage(avatar_location);
    if(!tex) {
        tex = Director::getInstance()->getTextureCache()->addImage(user_portraits[_local_profile->getAvatarIdx()]);
        return false;
    }
    return true;
}

void Game::friendDataCallback(const std::vector<std::map<std::string, std::string> > &friends_data)
{
    _local_profile->clearFacebookFriends();
    for ( auto& friend_data : friends_data) {
        const std::string& key = friend_data.at("id");
        if (key.empty()) continue;
        const std::string& pic = friend_data.at("picture");
        bool permission = false;
        /*if (friend_data.at("permissions").find("user_friends") != std::string::npos) {
            permission = true;
        }*/
        _local_profile->addFacebookFriend(std::pair<std::string, Profile::FacebookFriendData>(key,{pic, permission}));
    }
    incrementLoginStage(FACEBOOK_FRIEND_STAGE);
}

void Game::friendQueryComplete(bool success) {
    incrementLoginStage(PARSE_FRIEND_STAGE);
}

void Game::gameRequestDeletedCallback ( bool success, const std::shared_ptr<FacebookGameRequest>& req )
{
    for (auto delegate : _delegates) {
        delegate->clearInterupt ( GameDelegate::GameInteruptType::FACEBOOK, success, req->message );
    }
    //if (success) consumeGameRequest(req);
}

void Game::sendInterupt( )
{
    /*if (interupt == GameDelegate::GameInteruptType::REACHABILITY &&
        !Reachability::getInstance()->checkReachabilityForInternetConnection()){
        Facebook::getInstance()->clearGameRequests();
        for ( auto delegates : _delegates ) {
            delegates->recieveInterupt(GameDelegate::GameInteruptType::REACHABILITY);
        }
    } else if (!Facebook::getInstance()->isLoggedIn()) {
        Facebook::getInstance()->clearGameRequests();
        for ( auto delegates : _delegates ) {
            delegates->recieveInterupt(GameDelegate::GameInteruptType::LOGIN);
        }
    } else if (!Facebook::getInstance()->fetchGameRequests().empty()) {
        for ( auto delegates : _delegates ) {
            delegates->recieveInterupt(GameDelegate::GameInteruptType::FACEBOOK);
        }
    } else {*/
        if ( Game::getInstance( )->isInitalised( ) &&
             ParseWrapper::getInstance( )->isLoggedIn( ) &&
             Game::getInstance( )->getProfile( )->getFBPermission( ) )
             Game::getInstance( )->fetchFriendData( );
        for ( auto delegates : _delegates ) {
            delegates->recieveInterupt( GameDelegate::GameInteruptType::FOREGROUND );
        }
    //}
}

void Game::didShowInterupt (GameDelegate::GameInteruptType interupt) {
    Facebook::getInstance()->clearGameRequests();
}

void Game::gameRequestRecievedCallback      ( const std::set<std::string>& reqs )
{
}

void Game::gameRequestsCallback    ( bool success, const std::map<std::string, std::shared_ptr<FacebookGameRequest>>& reqs)
{
    if (success) {
        _local_profile->clearGameRequestData();
        for ( auto& req : reqs ) {
            FBGameRequestType type = req.second.get()->type;
            if ( type == FBGameRequestSend ||
                 type == FBGameRequestAskFor ) {
                _local_profile->addGameRequestData(req.second);
            }
        }
    }
    for ( auto delegate: _delegates) {
        delegate->gameRequestsComplete(success);
    }
    incrementLoginStage( REQUEST_STAGE );
}

void Game::textureLoadedCallback (Texture2D* tex) {
    for (auto delegate : _delegates) {
        delegate->textureDownloaded(tex);
    }
}

Texture2D* Game::downloadTexture(std::string &url, std::string &file)
{
    return _dlmanager->downloadTexture(url, file, CC_CALLBACK_1(Game::textureLoadedCallback, this));
}

void Game::errorCallback ( long error )
{
    if ( error == 8 ) {
        ParseWrapper::getInstance()->loginWithFacebook();
    } else {
        incrementLoginStage( USER_STAGE );
        incrementLoginStage( FACEBOOK_FRIEND_STAGE );
        incrementLoginStage( REQUEST_STAGE );
    }
}

void Game::userLoggedIn ( bool success )
{
    loginFinished();
}

void Game::userDataCallback ( const std::map<std::string,std::string>& user_data )
{
    for ( auto& i : user_data) {
               if (i.first.compare("name") == 0) {
            _local_profile->setUserName(i.second);
        } else if (i.first.compare("id") == 0) {
            _local_profile->setFBID(i.second);
        } else if (i.first.compare("email") == 0) {
            _local_profile->setEmail(i.second);
        } else if (i.first.compare("picture") == 0) {
            _local_profile->setAvatarUrl(i.second);
        } else if (i.first.compare("permissions") == 0) {
            if (i.second.find("user_friends") != std::string::npos) {
                _local_profile->setFBPermission(true);
            }
        }
    }
    incrementLoginStage( USER_STAGE );
}

void Game::dlUserPortrait ( )
{
    if (!_local_profile->getAvatarUrl().empty()) {
        auto tex = _dlmanager->downloadTexture(_local_profile->getAvatarUrl(), "avatar/" + _local_profile->getFBID() + ".jpg", [this] (Texture2D* tex) {
            incrementLoginStage( PORTRAIT_STAGE );
        });
        if (tex) incrementLoginStage( PORTRAIT_STAGE  );
    } else {
        incrementLoginStage( PORTRAIT_STAGE  );
    }
}

void Game::reachabilityStatusChanged( ReachabilityStatus status )
{
    _net_reachable = status;
}



bool Game::init ( )
{
    _dlmanager =    DownloadManager::create( );
    _table_data =   TableData::create( );
    setProfile(Profile::create());
    
    if (!_local_profile || !_dlmanager || !_table_data ) return false;
    
    
    ValueMap file = FileUtils::getInstance()->getValueMapFromFile("game.plist");
    ValueVector ranges = file.at("MapRanges").asValueVector();
    for ( Value range : ranges ) {
        _max_level += range.asInt();
        _map_ranges.push_back(range.asInt());
    }
    
    Facebook::getInstance()->addDelegate(this);
    Reachability::getInstance()->addDelegate(this);
    _local_profile->addDelegate(this);
    
    Reachability::getInstance()->monitorReachabilityWithHostName("http://www.google.com");
    _net_reachable = Reachability::getInstance()->checkReachabilityForInternetConnection();
    
    if (ParseWrapper::getInstance()->isLoggedIn()) {
        Facebook::getInstance()->refreshAccessToken();
        incrementLoginStage( PARSE_STAGE );
    }
    ParseWrapper::getInstance()->addDelegate(this);
    _dlmanager->retain();
    _table_data->retain();
    
    return true;
}

Sprite* Game::downloadCharacterPortrait(SaveClass* data)
{
    SaveObject* obj1 = data->getValue("FacebookId");
    SaveObject* obj2 = data->getValue("AvatarUrl");
    SaveObject* obj3 = data->getValue("AvatarIdx");

    
    if ( !obj1 || !obj2 || !obj3 ) return nullptr;
    
    TextureCache* tcache = Director::getInstance()->getTextureCache();
    
    std::string uuid =  static_cast<SaveValue<std::string>*>    (obj1)->getValue();
    std::string url =   static_cast<SaveValue<std::string>*>    (obj2)->getValue();
    uint16_t idx =      static_cast<SaveValue<uint16_t>*>       (obj3)->getValue();
    
    if ( idx > num_portraits ) idx = 0;
    
    std::string file_loc = uuid + ".jpg";

    Texture2D* tex = tcache->getTextureForKey(_dlmanager->getCacheLocation() + file_loc);
    if (tex) {
        return CharacterPortrait::create(tex, Game::getInstance()->getImageScale());
    }
    if (url.empty()) {
        return CharacterPortrait::create(tcache->getTextureForKey(user_portraits[idx]), 1.0f);
    }
    
    CharacterPortrait* portrait = CharacterPortrait::create(tcache->getTextureForKey(user_portraits[0]), 1.0f);
    Sprite* spinner = Sprite::create("load_spinner.png");
    spinner->runAction(RepeatForever::create(Sequence::create(
        RotateTo::create(0.25f, 90),
        RotateTo::create(0.25f, 180),
        RotateTo::create(0.25f, 270),
        RotateTo::create(0.25f, 360),
        NULL)));
    spinner->setPosition(portrait->getContentSize() * 0.5f);
    spinner->setPositionZ(64);
    spinner->setScale(0.8f);
    spinner->setOpacity(128);
    portrait->addChild(spinner);
    
    
    auto func = [this,idx,portrait,spinner] (Texture2D* tex) {
        float scale = Game::getInstance()->getImageScale();
        if (!tex) {
            scale = 1.0f;
            tex = Director::getInstance()->getTextureCache()->getTextureForKey(user_portraits[idx]);
        }
        portrait->setCharacterPortrait(tex, scale);
        portrait->cocos2d::Node::removeChild(spinner);
        for ( auto i : _delegates ) {
            i->portraitFinished(portrait);
        }
        portrait->release();
    };
    
    if (FileUtils::getInstance()->isFileExist(_dlmanager->getCacheLocation() + file_loc)) {
        portrait->retain();
        tcache->addImageAsync(_dlmanager->getCacheLocation() + file_loc, func);
    } else {
        portrait->retain();
        _dlmanager->downloadTexture(url, file_loc, func);
    }
    
    return portrait;
}

void Game::save ( bool cloud_save, bool cloud_check )
{
    ParseWrapper* parse = ParseWrapper::getInstance( );
    if ( parse->isLoggedIn() ) {
        if ( cloud_check ) {
            parse->saveUser( _local_profile->getUserData(), false );
            if ( cloud_save && _user_id == 0 ) {
                _user_id = parse->fetchUser( );
            }
        } else {
            parse->saveUser( _local_profile->getUserData(), true );
        }
    } else {
        _local_profile->save( );
    }

    if ( _profile_id == 0 ) {
        if (_local_profile->getUserId( ).empty( )) {
            _profile_id = parse->findObjectWithKey("Profile", "UUID", _local_profile->getUUID());
        } else {
            std::shared_ptr<SaveClass> profile = _local_profile->copyUserData();
            profile->setKey("Profile");
            parse->saveWithObjectId(profile, _local_profile->getUserId( ));
        }
    }
}

void Game::parseObjectSaved ( const std::shared_ptr<ParseInterfaceObject>& obj, int query_id, bool success )
{
    if ( success ) {
        if ( obj->getParseObject()->getClassName().compare("Profile") == 0 ) {
            if (_local_profile->getUserId( ).empty( )) {
                _local_profile->setUserId(obj->getParseObject()->getObjectId());
                save( false, false );
            }
            _profile_id = 0;
        } else if ( obj->getParseObject()->getClassName().compare("_User") == 0 ) {
            SaveValue<Date>* date = static_cast<SaveValue<Date>*>(_local_profile->getUserData()->getValue("SaveTime"));
            date->setValue(Date(obj->getParseObject()->getUpdatedAt()));
        }
    }

    if ( obj->getParseObject()->getClassName().compare("_User") == 0 ) {
        for ( auto i : _delegates ) {
            i->userSaved(success);
        }
    }
}

bool Game::isIncomingURL           ( std::string& url, std::string& source )
{
    Facebook* fb = Facebook::getInstance();
    if (source.find("facebook") && fb->parseURL(url)) {
        return true;
    }
    return false;
}

void Game::setProfile ( Profile* profile )
{
    if (_local_profile) {
        profile->transitionProfile(_local_profile);
        _local_profile->release();
    }
    _local_profile = profile;
    _local_profile->retain( );
    ParseWrapper::getInstance()->addDelegate(_local_profile);
}


void Game::parseLoginCallback ( ParseDelegate::LoginState success, bool new_user )
{
    if (new_user)   CFabric::CAnswers::logSignUpWithMethod("Facebook", success == SUCCESS);
    else            CFabric::CAnswers::logLoginWithMethod("Facebook", success == SUCCESS);
    
    if (success == SUCCESS) {
        reset();
        setProfile(Profile::create());
        load( );
        _local_profile->setNewUser(new_user);
        incrementLoginStage( PARSE_STAGE );
        loginFinished( );
    } else {
        _state = 0;
    }
}

void Game::reset()
{
    _state = 0;
    _login = false;
    _profile_id = 0;
    _query_id = 0;
    _profile_results.clear();
}

void Game::parseLogOutCallback( )
{
    reset();
    Profile* old_profile = _local_profile;
    old_profile->retain();
    
    setProfile(Profile::create( ));
    load( );
    
    for ( auto i : _delegates ) {
        i->gameLogOut( old_profile );
    }
    CC_SAFE_RELEASE(old_profile);
}

void Game::parseQueryComplete( int query_id, bool success )
{
    ParseWrapper* parse = ParseWrapper::getInstance();
    
    if ( _profile_id == query_id ) {
        if (success) {
            std::shared_ptr<ParseInterfaceObject> profile;
            time_t counter = 0;
            if (_profile_results.size() > 0) {
                for ( auto i : _profile_results ) {
                    Date update_time = i->getParseObject()->getUpdatedAt();
                    if (update_time.getTime() > counter) profile = i;
                }
                for ( auto i : _profile_results ) {
                    if (i != profile) {
                        parse->deleteWithObjectId("Profile", i->getParseObject()->getObjectId());
                    }
                }
                _profile_results.clear();
            }
            if (profile) {
                _local_profile->setUserId(profile->getParseObject()->getObjectId());
            }
            //std::shared_ptr<SaveClass> new_profile = _local_profile->createProfileData();
            std::shared_ptr<SaveClass> new_profile = _local_profile->copyUserData();
            new_profile->setKey("Profile");
            if (profile) {
                parse->saveWithObjectId( new_profile, _local_profile->getUserId() );
                _profile_id = 0;
            } else {
                parse->saveNewObject( new_profile );
            }
        }
    }
    // map data
    else if ( _query_id == query_id ) {
        _query_complete = true;
        for ( auto i : _delegates ) {
            i->mapQueryComplete(success);
        }
    }
    // before we save to server we attempt to verify the data is still in sync with our cloud data. If someone uses another device with the same account our data may need to be merged
    else if ( _user_id == query_id ) {
        const std::shared_ptr<SaveClass>& local_data = _local_profile->getUserData();
        const std::shared_ptr<SaveClass>& cloud_data = _cloud_profile->getSaveObject();
        SaveValue<Date>* local_date = static_cast<SaveValue<Date>*>( local_data->getValue("SaveTime") );
        if ( _cloud_profile && local_date ) {
            Date cloud_date(_cloud_profile->getParseObject()->getUpdatedAt());
            if ( cloud_date != local_date->getValue() ) {
                for ( auto i : _delegates ) {
                    i->saveMismatch( cloud_data, local_data );
                }
            } else {
                parse->saveUser( local_data, true );
                clearCloudProfile();
            }
        }
    }
}

void Game::clearCloudProfile()
{
    if (_cloud_profile.get()) {
        _cloud_profile.reset();
    }
    _user_id = 0;
}

void Game::parseQueryResult( const std::shared_ptr<ParseInterfaceObject>& obj, int query_id )
{
    if ( _query_id == query_id ) {
        _query_data.push_back(obj);
    } else if ( _profile_id == query_id ) {
        _profile_results.push_back(obj);
    } else if ( _user_id == query_id ) {
        _cloud_profile = obj;
    }
    
}

std::shared_ptr<SaveClass> Game::parseQueryCallback ( int query_id )
{
    if ( query_id == _query_id || query_id == _profile_id ) {
        std::shared_ptr<SaveClass> profile (_local_profile->createProfileQuery());
        return profile;
    } else if ( query_id == _user_id ) {
        return Profile::createUserData();
    }
    
    return nullptr;
}


void Game::load ( )
{
    ParseWrapper* parse = ParseWrapper::getInstance();

    if (parse->isLoggedIn( )) {
        std::shared_ptr<ParseInterfaceObject> obj = parse->loadUser(_local_profile->getUserData( ));
        SaveValue<Date>* date = static_cast<SaveValue<Date>*>(obj->getSaveObject()->getValue("SaveTime"));
        date->setValue(Date(obj->getParseObject()->getUpdatedAt()));
    } else {
        _local_profile->load( );
    }
}

Vec2 Game::getLevelRangeForMap( uint8_t map_idx )
{
    uint8_t min = 0;
    uint8_t max = 0;
    uint16_t count = 1;
    for ( auto i : _map_ranges ) {
        min = max + 1;
        max += i;
        if (count >= map_idx) break;
        count++;
    }
    return Vec2(min,max);
}


void Game::appInviteComplete       ( bool success )
{
    if (success) CFabric::CAnswers::logInviteWithMethod("Facebook");
}


void Game::queryMapScores( )
{
    Vec2 map_range = getLevelRangeForMap(getMapIndex());
    _query_complete = false;
    _query_id = ParseWrapper::getInstance()->getMapScores(getProfile()->getUUID(),
                                                          getProfile()->getLanguage(),
                                   map_range.x,
                                   map_range.y,
                                   !isNetworkReachable());
}

void Game::parseQueryStarted ( int query_id, bool success )
{
    if ( _query_id == query_id && success ) {
        _query_data.clear();
    }
}


void Game::queryFriendData()
{
    _local_profile->queryFriendData();
}

uint8_t Game::findMapIdxWithLevelIdx ( uint16_t level )
{
    uint8_t idx = 1;
    uint16_t lvl_count = 0;
    for ( uint8_t range : _map_ranges ) {
        lvl_count += range;
        if ( level < lvl_count ) {
            break;
        }
        idx++;
    }
    return idx;
}

uint8_t Game::getCurrentLevelMap( )
{
    return findMapIdxWithLevelIdx(_local_profile->getCurrentLevel());
}

uint16_t Game::getMaxMapIndex( )
{
    return findMapIdxWithLevelIdx(_local_profile->getMaxLevel( ));
}
