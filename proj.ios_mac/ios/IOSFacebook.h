//
//  IOSFacebook.h
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#ifndef __Pirateer__IOSFacebook__
#define __Pirateer__IOSFacebook__

#include "../Classes/external/Facebook.h"

class IOSFacebook : public Facebook
{
public:
    
    CREATE_FUNC(IOSFacebook);
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
    
    virtual void getUserData            ( );
    virtual void getFriendData          ( );
    virtual void getGameRequests        ( const std::string& app_id );
    virtual void refreshAccessToken     ( );

    virtual bool isLoggedIn             ( ) const;
    
private:
    virtual ~IOSFacebook                ( );
    

    void performPublishAction           ( );
    void publishActionComplete          ( );
    
};

#endif /* defined(__Pirateer__IOSFacebook__) */
