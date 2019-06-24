//
//  Facebook.cpp
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#include "Facebook.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "../../proj.ios_mac/ios/IOSFacebook.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../../proj.android/app/jni/AndroidFacebook.h"

#endif

static Facebook* s_Facebook = NULL;

void FacebookDelegate::userLoggedIn                 ( bool success ) {}
void FacebookDelegate::userLoggedOut                ( ) {}
void FacebookDelegate::userPosted                   ( ) {}
void FacebookDelegate::gameRequestComplete          ( bool success ) {}
void FacebookDelegate::appInviteComplete            ( bool success ) {}
void FacebookDelegate::errorCallback                ( long error ) {}
void FacebookDelegate::shareCallback                ( bool success ) {}
void FacebookDelegate::userDataCallback             ( const std::map<std::string,std::string>& user_data ) {}
void FacebookDelegate::friendDataCallback           ( const std::vector<std::map<std::string,std::string>>& friends_data ) {}
void FacebookDelegate::gameRequestDeletedCallback   ( bool success, const std::shared_ptr<FacebookGameRequest>& req ) {}
void FacebookDelegate::gameRequestsCallback         ( bool success, const std::map<std::string,
                                                      std::shared_ptr<FacebookGameRequest>>& req) {}
void FacebookDelegate::gameRequestRecievedCallback  ( const std::set<std::string>& reqs ) {}




Facebook::Facebook()
{
    
}

Facebook::~Facebook()
{
}

Facebook* Facebook::getInstance() {
    if (!s_Facebook) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        s_Facebook = new IOSFacebook();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        s_Facebook = new AndroidFacebook();
#else
        s_Facebook = new Facebook();
#endif
    s_Facebook->init();
    }
    return s_Facebook;
}

void Facebook::purgeFacebook()
{
    CC_SAFE_DELETE(s_Facebook);
    s_Facebook = NULL;
}


bool Facebook::init()
{
    return true;
}

bool Facebook::parseURL(const std::string &url)
{
    CCLOG("%s",url.c_str());
    _game_reqs.clear();
	if (url.find("facebook")) {
        auto start_pos = url.find("request_ids=");
        if (start_pos != std::string::npos ) {
            start_pos += std::string("request_ids=").length();
            auto end_pos = url.find("&",start_pos);
            if (end_pos != std::string::npos) {
                auto str = url.substr(start_pos,end_pos - start_pos);
                start_pos = 0;
                while (1) {
                    str = str.substr(start_pos);
                    start_pos = str.find(",");
                    auto request_id = str.substr(0,start_pos);
                    _game_reqs.insert(request_id);
                    if (start_pos == std::string::npos) break;
                    start_pos++;
                }
                gameRequestRecievedCallback(_game_reqs);
            }
            return true;
        }
        
    }
    
    return false;
}


void Facebook::userLoggedIn( bool success )
{
    _game_reqs.clear();
    for ( auto i : _delegates )
    {
        i->userLoggedIn( success );
    }
}

void Facebook::userLoggedOut()
{
    _game_reqs.clear();
    for ( auto i : _delegates )
    {
        i->userLoggedOut();
    }
}


void Facebook::removeDelegate (FacebookDelegate* delegate)
{
    _delegates.erase(std::remove(_delegates.begin(), _delegates.end(), delegate), _delegates.end());
}

void Facebook::userPosted()
{
    for ( auto i : _delegates ) {
        i->userPosted();
    }
}

void Facebook::errorCallback ( long error )
{
    for ( auto i : _delegates ) {
        i->errorCallback(error);
    }
}

std::string Facebook::pictureUrl( const std::string& fbid )
{
    return "https://graph.facebook.com/" + fbid + "/picture?width=200&height=200";
}
void Facebook::userDataCallback( const std::map<std::string,std::string>& me_data )
{
    for ( auto i : _delegates ) {
        i->userDataCallback(me_data);
    }
}

void Facebook::friendDataCallback( const std::vector<std::map<std::string, std::string> > &friends_data )
{
    for ( auto i : _delegates ) {
        i->friendDataCallback(friends_data);
    }
}

void Facebook::shareComplete ( bool success )
{
    for ( auto i : _delegates ) {
        i->shareCallback(success);
    }
}

void Facebook::gameRequestsCallback( bool success, const std::map<std::string, std::shared_ptr<FacebookGameRequest> >& req )
{
    for ( auto i : _delegates ) {
        i->gameRequestsCallback( success, req );
    }
}

void Facebook::gameRequestDeletedCallback( bool success,
                                           const std::shared_ptr<FacebookGameRequest>& req )
{
    for ( auto i : _delegates ) {
        i->gameRequestDeletedCallback( success, req );
    }
}

void Facebook::gameRequestRecievedCallback( const std::set<std::string>& reqs )
{
    for ( auto i : _delegates ) {
        i->gameRequestRecievedCallback(reqs);
    }
}

void Facebook::appInviteComplete ( bool success )
{
    for ( auto i : _delegates ) {
        i->appInviteComplete(success);
    }
}

void Facebook::gameRequestComplete ( bool success )
{
    for ( auto i : _delegates ) {
        i->gameRequestComplete(success);
    }
}





