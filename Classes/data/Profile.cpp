//
//  Player.cpp
//  Pirateer
//
//  Created by Peter Respondek on 1/24/14.
//
//

#include "../data/Game.h"
#include "../cocos_extensions/Lanyard_Util.h"
#include "../data/ElixirLocalization.h"
#include "../cocos_extensions/Lanyard_Scheduler.h"
#include "../ElixirTypes.h"
#include "../scenes/RootScene.h"
#include "../external/PlatformUtils.h"
#include "../external/Notifications.h"

PlayerPrefs::PlayerPrefs() :
m_volume(1.0f)
{
}

PlayerPrefs::~PlayerPrefs()
{
}

bool PlayerPrefs::init()
{
    return true;
}

void PlayerPrefs::updatePrefs()
{
    
    
}

PlayerStats::PlayerStats()

{
  
}



PlayerStats::~PlayerStats()
{
}

bool PlayerStats::init()
{
    
    return true;
}


uint16_t Profile::getAvatarIdx       ( )
{
    uint16_t idx = _avatar_idx->getValue();
    if ( idx < 0 && idx > num_portraits) {
        idx = 0;
    }
    return idx;
}


Profile::Profile() :
_user_data          (nullptr),
_parse_friends      (nullptr),
_user_id            (nullptr),
_level_scores       (nullptr),
_ambrosia           (nullptr),
_lives              (nullptr),
_level              (nullptr),
_avatar_idx         (nullptr),
_max_level          (nullptr),
_max_lives          (nullptr),
_extra_lives        (nullptr),
_time_stamp_total   (nullptr),
_time_stamp_save    (nullptr),
_regen_level        (nullptr),
_user_name          (nullptr),
_avatar_url         (nullptr),
_uuid               (nullptr),
_email              (nullptr),
_language           (nullptr),
_notification       (false),
_new_user           (false),
_friend_query_id    (UINT16_MAX),
_fb_query_time      (0),
_time_stamp_session (time(NULL)),
_debug_flags        (0),
_fb_permission      (false)

{
}

Profile::~Profile()
{
    Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
    ParseWrapper::getInstance()->removeDelegate(this);
}

std::shared_ptr<SaveClass> Profile::createUserData()
{
    std::shared_ptr<SaveClass> user_data(new SaveClass      ("User"));
    user_data->temporary = true;
    user_data->push(new SaveValue<std::string>              ("UserId"));
    user_data->push(new SaveValue<std::string>              ("FacebookId"));
    user_data->push(new SaveValue<std::set<std::string>>    ("FBFriends"));
    user_data->push(new SaveList<uint32_t>                  ("LevelScore"));
    user_data->push(new SaveValue<uint16_t>                 (50,"Ambrosia")),
    user_data->push(new SaveList<time_t>                    ("Lives"));
    user_data->push(new SaveValue<uint16_t>                 (1,"Level"));
    user_data->push(new SaveValue<uint16_t>                 (0,"AvatarIdx"));
    user_data->push(new SaveValue<uint16_t>                 (1,"MaxLevel"));
    user_data->push(new SaveValue<uint16_t>                 (3,"MaxLives"));
    user_data->push(new SaveValue<uint16_t>                 (0,"ExtraLives"));
    user_data->push(new SaveValue<time_t>                   (time(NULL), "TimeStampTotal"));
    user_data->push(new SaveValue<Date>                     (0,"SaveTime"));
    user_data->push(new SaveValue<uint16_t>                 (0,"RegenLevel"));
    user_data->push(new SaveValue<std::string>              ("Name"));
    user_data->push(new SaveValue<std::string>              ("", "AvatarUrl", false));
    user_data->push(new SaveValue<std::string>              ("UUID"));
    user_data->push(new SaveValue<std::string>              ("Email"));
    user_data->push(new SaveValue<std::string>              ("Purchase"));
    user_data->push(new SaveValue<uint16_t>                 (uint16_t(Application::getInstance()->getCurrentLanguage()),"Language"));
    
    return user_data;
}

void Profile::setUserName        ( const std::string& name ) {
    _user_name->setValue(name);
    _language->setValue((uint16_t)Application::getInstance()->getCurrentLanguage());
}

 void                 Profile::unlockLevel        ( ) {
    _max_level->setValue(_max_level->getValue() + 1);
}

void Profile::setUserData(const std::shared_ptr<SaveClass>& user_data)
{
    _user_data =        user_data;
    _parse_friends =    static_cast<SaveValue<std::set<std::string>>*>  (_user_data->getValue("FBFriends"));
    _user_id =          static_cast<SaveValue<std::string>*>            (_user_data->getValue("UserId"));
    _fb_id =            static_cast<SaveValue<std::string>*>            (_user_data->getValue("FacebookId"));
    _level_scores =     static_cast<SaveList<uint32_t>*>                (_user_data->getValue("LevelScore"));
    _ambrosia =         static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("Ambrosia"));
    _lives =            static_cast<SaveList<time_t>*>                  (_user_data->getValue("Lives"));
    _level =            static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("Level"));
    _avatar_idx =       static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("AvatarIdx"));
    _max_level =        static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("MaxLevel"));
    _max_lives =        static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("MaxLives"));
    _extra_lives =      static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("ExtraLives"));
    _time_stamp_total = static_cast<SaveValue<time_t>*>      (_user_data->getValue("TimeStampTotal"));
    _time_stamp_save =  static_cast<SaveValue<Date>*>                   (_user_data->getValue("SaveTime"));
    _regen_level =      static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("RegenLevel"));
    _user_name =        static_cast<SaveValue<std::string>*>            (_user_data->getValue("Name"));
    _avatar_url =       static_cast<SaveValue<std::string>*>            (_user_data->getValue("AvatarUrl"));
    _uuid =             static_cast<SaveValue<std::string>*>            (_user_data->getValue("UUID"));
    _email =            static_cast<SaveValue<std::string>*>            (_user_data->getValue("Email"));
    _pending_purchase = static_cast<SaveValue<std::string>*>            (_user_data->getValue("Purchase"));
    _language =         static_cast<SaveValue<uint16_t>*>               (_user_data->getValue("Language"));
}

std::shared_ptr<SaveClass> Profile::copyUserData()
{
    return std::shared_ptr<SaveClass>(static_cast<SaveClass*>(_user_data->copy()));
}

std::shared_ptr<SaveClass> Profile::createProfileQuery()
{
    std::shared_ptr<SaveClass> obj (new SaveClass( "Profile" ));
    obj->temporary = true;
    obj->push(new SaveList<uint32_t>      ("LevelScore"));
    obj->push(new SaveValue<std::string>  ("",  "UUID"));
    obj->push(new SaveValue<std::string>  ("",  "FacebookId"));
    obj->push(new SaveValue<std::string>  ("",  "Name"));
    obj->push(new SaveValue<uint16_t>     (0,   "Level"));
    obj->push(new SaveValue<uint16_t>     (0,   "AvatarIdx"));
    obj->push(new SaveValue<std::string>  ("",  "AvatarUrl", false));
    obj->push(new SaveValue<uint16_t>     (0,   "MaxLevel"));
    obj->push(new SaveList<time_t>        ("Lives"));
    
    return obj;
}


void Profile::queryFriendData( )
{
    std::vector<std::string> ids;
    for (auto& i : _parse_friends->getValue()) {
        ids.push_back(i);
    }
    for (auto& i : _fb_friends) {
        ids.push_back(i.first);
    }
    _friend_query_id = ParseWrapper::getInstance()->getFriends(ids, !Game::getInstance()->isNetworkReachable());
}

std::shared_ptr<SaveClass> Profile::parseQueryCallback              ( int query_id  )
{
    if ( query_id == _friend_query_id ) {
        return createProfileQuery();
    }
    return nullptr;
        
}

void Profile::parseQueryStarted  ( int query_id, bool success )
{
    if ( query_id == _friend_query_id && success ) {
        _parse_query_time = time(NULL);
        _friend_data.clear();
    }
}

void Profile::parseQueryComplete              ( int query_id, bool success )
{
    if ( query_id == _friend_query_id ) {
        for (auto i : _friend_data) {
            auto uuid = static_cast<SaveValue<std::string>*>(i->getSaveObject()->getValue("FacebookId"));
            auto url = static_cast<SaveValue<std::string>*>(i->getSaveObject()->getValue("AvatarUrl"));
            auto iter = _fb_friends.find(uuid->getValue());
            if ( iter != _fb_friends.end() ) {
                url->setValue(Facebook::getInstance()->pictureUrl(uuid->getValue()));
            }
        }
        for (auto i : _delegates) {
            i->friendQueryComplete(success);
        }
    }
}

void Profile::parseQueryResult                ( const std::shared_ptr<ParseInterfaceObject>& result, int query_id )
{
    if ( query_id == _friend_query_id ) {
        _friend_data.push_back(result);
    }
}

void Profile::saveValue ( const std::string& value )
{
    UserDefault* ud = UserDefault::getInstance();
    ud->setStringForKey(_key.c_str(), value);
    
}

void Profile::saveValue ( const unsigned int& value )
{
    UserDefault* ud = UserDefault::getInstance();
    ud->setIntegerForKey(_key.c_str(), value);
}

void Profile::saveValue ( const unsigned short& value )
{
    UserDefault* ud = UserDefault::getInstance();
    ud->setIntegerForKey(_key.c_str(), value);

}

void Profile::saveValue ( const long& value )
{
    UserDefault* ud = UserDefault::getInstance();
    ud->setStringForKey(_key.c_str(), cocos2d::StringUtils::toString(value));
}

void Profile::saveValue ( const Date& value )
{
    UserDefault* ud = UserDefault::getInstance();
    ud->setStringForKey(_key.c_str(), cocos2d::StringUtils::toString(value.getTime()));
}

void Profile::saveValue ( const std::set<std::string> &value )
{
    UserDefault* ud = UserDefault::getInstance();
    uint16_t counter = 0;
    for ( auto& i : value ) {
        ud->setStringForKey((_key + cocos2d::StringUtils::toString(counter)).c_str(),i);
        counter++;
    }
    ud->setStringForKey((_key + "Count").c_str(),
                        cocos2d::StringUtils::toString(value.size()));
}

void Profile::loadValue ( std::set<std::string> &value )
{
    UserDefault* ud = UserDefault::getInstance();
    long counter = atol(ud->getStringForKey((_key + "Count").c_str(),"0").c_str());
    for ( long i = 0;  i < counter; i++ ) {
        value.insert(ud->getStringForKey((_key + cocos2d::StringUtils::toString(i)).c_str(),""));
    }
}

void Profile::loadValue ( std::string& def )
{
    UserDefault* ud = UserDefault::getInstance();
    def = ud->getStringForKey(_key.c_str(), def);
}

void Profile::loadValue ( unsigned int& def )
{
    UserDefault* ud = UserDefault::getInstance();
    def = ud->getIntegerForKey(_key.c_str(), def);
}

void Profile::loadValue ( unsigned short& def )
{
    UserDefault* ud = UserDefault::getInstance();
    def = ud->getIntegerForKey(_key.c_str(), def);
}

void Profile::loadValue ( long& def )
{
    UserDefault* ud = UserDefault::getInstance();
    std::string str = ud->getStringForKey(_key.c_str(), CCLanyard_Util::toString(def));
    def = atol(str.c_str());
}

void Profile::loadValue ( Date& def )
{
    UserDefault* ud = UserDefault::getInstance();
    std::string str = ud->getStringForKey(_key.c_str(), CCLanyard_Util::toString(def.getTime()));
    def.setTime(atol(str.c_str()));
}

void Profile::copyProfile(Profile *profile)
{
    _avatar_idx =   profile->_avatar_idx;
    _level =        profile->_level;
    _max_level =    profile->_max_level;
    _ambrosia =     profile->_ambrosia;
    _max_lives =    profile->_max_lives;
    _level_scores = profile->_level_scores;
    _lives =        profile->_lives;
    _max_lives =    profile->_max_lives;
    _regen_level =  profile->_regen_level;
    _delegates =    profile->_delegates;
                    profile->_delegates.clear();
}

void Profile::transitionProfile(Profile *profile)
{
    for ( auto i : profile->_delegates ) {
        addDelegate(i);
    }
    profile->_delegates.clear();
    _debug_flags = profile->_debug_flags;
}

void Profile::save( )
{
    saveObject(_user_data.get());
}

void Profile::load( )
{
    _debug_flags = UserDefault::getInstance()->getIntegerForKey("DebugFlags", 0);
    loadObject(_user_data.get());
}

void Profile::removeLife() {
    if (getDebugFlag(Profile::INFINITE_LIVES))
        return;
    if ( _lives->getValues().size() < _max_lives->getValue()) {
        _lives->push(time(NULL));
    } else if ( _extra_lives->getValue() > 0 ) {
        _extra_lives->setValue( _extra_lives->getValue() - 1 );
    }
    postLocalNotifications();
}

void Profile::postLocalNotifications()
{
    Notifications::cancelLocalNotifications();
    if ( _lives->getValues().size() ) {
        if (_lives->getValues().size() > 1) {
    Notifications::postLocalNotification(*std::max_element(
                                                           _lives->getValues().begin(),
                                                           _lives->getValues().end()),
                                         LOCALIZE("Life restored"));
        }
    Notifications::postLocalNotification(*std::min_element(
                                                           _lives->getValues().begin(),
                                                           _lives->getValues().end()),
                                         LOCALIZE("All lives restored"));
    }
}

uint8_t  Profile::fetchDebugPlays      ( )
{
    return _debug_flags >> 12;
}

uint8_t  Profile::getDebugPlays      ( )
{
    uint8_t plays = fetchDebugPlays();
    if (plays > 1) plays = (plays - 1) * 5;
    return plays;
}

void  Profile::setDebugPlays      ( uint16_t plays )
{
    if (plays > 16) plays = 16;
    _debug_flags = _debug_flags << 4;
    _debug_flags = _debug_flags >> 4;
    plays = plays << 12;
    _debug_flags |= plays;
}

inline uint16_t             Profile::getRegenTime       ( ) {
    uint16_t regen_time = REGEN_TIME;
    for (uint16_t i = 0;i < _regen_level->getValue();i++){
        regen_time *= 0.75;
    }
    return regen_time;
}

void Profile::update (float dt)
{
    uint16_t regen_time = getRegenTime();
    time_t min_time = 0;
    time_t curr_time = time(NULL);
    std::vector<time_t>& lives = _lives->getValues();
    for ( auto i = lives.begin(); i != lives.end(); ) {
        time_t delta = difftime(curr_time,*i);
        if (delta > regen_time) {
            lives.erase(i);
            for (auto i : _delegates) {
                i->lifeRestored();
            }
        } else {
            min_time = MAX(delta,min_time);
            i++;
        }
    }
    if (_lives->getValues().size() >= _max_lives->getValue()) {
        for (auto i : _delegates) {
            i->regenTimeRemaining(regen_time - min_time);
        }
    }
}

bool Profile::init()
{
    setUserData(createUserData());
    std::string uuid;
    PlatformUtils::getInstance()->createUUID(uuid);
    _uuid->setValue(uuid);
    Director::getInstance()->getScheduler()->schedule(CC_CALLBACK_1(Profile::update, this), this, 1.0f, CC_REPEAT_FOREVER, 0.0f, false, "profile");
    return true;
}

void Profile::setAmbrosia        ( uint16_t amb )
{
    uint16_t old = _ambrosia->getValue();
    _ambrosia->setValue(amb);
    for ( auto delegate : _delegates) {
        delegate->ambrosiaChanged(old);
    }
}


void Profile::addGameRequestData(const std::shared_ptr<FacebookGameRequest>& req)
{
    auto iter = _game_reqs.find(req->from_id);
    if (iter == _game_reqs.end()) {
        iter = _game_reqs.insert(std::pair<std::string, std::vector<std::shared_ptr<FacebookGameRequest>>>(req->from_id, std::vector<std::shared_ptr<FacebookGameRequest>>())).first;
        iter->second.push_back(req);
        for ( auto delegate : _delegates ) {
            delegate->requestsUpdated();
        }
    }
}

std::shared_ptr<SaveClass> Profile::getFriendDataWithUUID ( const std::string& user_id )
{
    for ( auto i : _friend_data ) {
        SaveObject* obj = i->getSaveObject()->getValue("FacebookId");
        if (obj && obj->getValueAsString().compare(user_id) == 0) {
            return i->getSaveObject();
        }
    }
    return nullptr;
}

void Profile::addLife ( )
{
    _max_lives->setValue(_max_lives->getValue() + 1);
    for (auto i : _delegates) {
        i->lifeRestored();
    }
    
}

void Profile::addExtraLife ( )
{
    _extra_lives->setValue(_extra_lives->getValue() + 1);
    for (auto i : _delegates) {
        i->lifeRestored();
    }
}

void Profile::restoreLife()
{
    time_t min_time = INT64_MAX;
    std::vector<time_t>& lives = _lives->getValues();
    auto j = lives.end();
    for ( auto i = lives.begin(); i != lives.end(); i++) {
        time_t delta = difftime(time(NULL), *i);
        if (delta <= min_time) {
            j = i;
            min_time = delta;
        }
    }
    if (j != lives.end()) {
        lives.erase(j);
        for (auto i : _delegates) {
            i->lifeRestored();
        }
    }
}

uint32_t Profile::getLevelScore( uint16_t lvl )
{
    if (_level_scores->getValues().size() < lvl) return 0;
    return _level_scores->getValues().at(lvl-1);
}

void Profile::setLevelScore( uint16_t lvl, uint32_t score )
{
    if (_level_scores->getValues().size() < lvl) _level_scores->getValues().resize(lvl, 0);
    _level_scores->getValues().at(lvl - 1) = score;
}

void Profile::addDelegate                            (ProfileDelegate* delegate)
{
    CCASSERT(delegate, "NULL delegate");
    _delegates.push_back(delegate);
}

void Profile::removeDelegate                         (ProfileDelegate* delegate)
{
    auto find = std::find(_delegates.begin(), _delegates.end(), delegate);
    if (find != _delegates.end()) {
        _delegates.erase(find);
    }
}

void Profile::getPendingRequest( std::shared_ptr<FacebookGameRequest>& rec )
{
    for ( auto& req : _game_reqs ) {
        for ( auto& req_ids : req.second) {
            for (auto& req_id : req_ids->req_ids) {
                std::string rec_id = req_id.substr(0,req_id.find("_"));
                for ( auto& pending_req : Facebook::getInstance()->fetchGameRequests( ) ) {
                    if ( pending_req == rec_id ) {
                        rec = req_ids;
                    }
                }
            }
        }
    }
}

void Profile::removeGameRequest ( const std::shared_ptr<FacebookGameRequest>& req )
{
    for ( auto& game_req : _game_reqs ) {
        auto iter = std::find( game_req.second.begin(), game_req.second.end(), req );
        if (iter != game_req.second.end()) {
            game_req.second.erase(iter);
            for ( auto delegate : _delegates ) {
                delegate->requestsUpdated();
            }
        }
    }
}

size_t Profile::getNumGameRequests ( )
{
    size_t counter = 0;
    for ( auto& game_req : _game_reqs ) {
        auto iter = _fb_friends.find(game_req.first);
        if (iter != _fb_friends.end())
            counter += game_req.second.size();
    }
    return counter;
}

const std::vector<std::shared_ptr<FacebookGameRequest>>* Profile::getFriendRequests(const std::string &uuid)
{
    auto iter = _game_reqs.find(uuid);
    if (iter != _game_reqs.end()) {
        return &iter->second;
    }
    return nullptr;
}
