//
//  IOSFacebook.h
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#ifndef __Pirateer__IOSFacebook__
#define __Pirateer__IOSFacebook__

#include "external/Facebook.h"

class AndroidFacebook : public Facebook
{
public:
    
    CREATE_FUNC(AndroidFacebook);
    AndroidFacebook						( );
    static AndroidFacebook* get			( );
    bool init ();

    virtual void logIn                  ( );
    virtual void logOut                 ( );
    virtual void postStatusUpdate       ( const std::string& caption );
    virtual void postLink               ( const std::string& link );
    virtual void openAppInviteDialog    ( const std::string&, const std::string& );
    virtual void sendGameRequest        ( FBGameRequestType type,
                                          const std::string& message,
                                          const std::string& title,
                                          const std::string& recipient );
    virtual void deleteGameRequest      ( const std::shared_ptr<FacebookGameRequest>& req );
    virtual bool parseURL               ( const std::string& url );
    virtual void errorCallback  		( int code );
    virtual void getUserData            ( );
    virtual void getFriendData          ( );
    virtual void refreshAccessToken		( );
    virtual void getGameRequests        ( const std::string& app_id );

    virtual bool isLoggedIn             ( ) const;
    virtual void userLoggedIn           ( bool success );
    virtual void friendDataCallback     ( const std::vector<std::map<std::string,std::string>>& friends_data );
    virtual void userDataCallback 		( const std::map<std::string,std::string>& user_data );

    void gameRequestRemoved				( std::string req );
    void gameRequestRemoveComplete		( );

protected:
    virtual ~AndroidFacebook                ( );

    const char* const CLASS_NAME = 			"com/bridge/facebook/FacebookBridge";
    std::shared_ptr<FacebookGameRequest> 	_game_request;
    uint32_t 								_req_count;
};

#endif /* defined(__Pirateer__IOSFacebook__) */
