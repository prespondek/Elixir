//
//  Facebook.h
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#ifndef __Pirateer__Facebook__
#define __Pirateer__Facebook__

#include "cocos2d.h"
USING_NS_CC;

enum FBGameRequestType
{
    FBGameRequestNone = 0,
    FBGameRequestSend,
    FBGameRequestAskFor,
    FBGameRequestTurn
};


struct FacebookGameRequest
{
    FBGameRequestType type;
    std::string message;
    std::string from_id;
    std::string from_name;
    std::vector<std::string> req_ids;
};

class FacebookDelegate
{
    friend class Facebook;

protected:
    virtual void userLoggedIn                   ( bool success );
    virtual void userLoggedOut                  ( );
    virtual void userPosted                     ( );
    virtual void appInviteComplete              ( bool success );
    virtual void gameRequestComplete            ( bool success );
    virtual void shareCallback                  ( bool success );
    virtual void errorCallback                  ( long error );
    virtual void userDataCallback               ( const std::map<std::string,std::string>& me_data );
    virtual void friendDataCallback             ( const std::vector<std::map<std::string,std::string>>& friends_data );
    virtual void gameRequestsCallback           ( bool success,
                                                  const std::map<std::string, std::shared_ptr<FacebookGameRequest>>& req );
    virtual void gameRequestDeletedCallback     ( bool success, const std::shared_ptr<FacebookGameRequest>& req );
    virtual void gameRequestRecievedCallback    ( const std::set<std::string>& reqs );
};

class Facebook : public Ref
{
public:
    virtual ~Facebook();

    static Facebook* getInstance                ( );
    static std::string pictureUrl               ( const std::string& fbid );
    void purgeFacebook                          ( );
    virtual bool init                           ( );
    virtual void logIn                          ( ) {}
    virtual void logOut                         ( ) {}
    virtual void refreshAccessToken             ( ) {}
    virtual void getPermissions                 ( const std::string& = "") {}
    virtual void postStatusUpdate               ( const std::string& ) {}
    virtual void postLink                       ( const std::string& link ) {}
    virtual void openAppInviteDialog            ( const std::string&, const std::string& img_url ) {}
    virtual bool parseURL                       ( const std::string& url );
    virtual void sendGameRequest                ( FBGameRequestType type,
                                                  const std::string& message,
                                                  const std::string& title,
                                                  const std::string& recipient ) {}
    virtual void deleteGameRequest              ( const std::shared_ptr<FacebookGameRequest>& req ) {}
    
    virtual void getUserData                    ( ) {}
    virtual void getFriendData                  ( ) {}
    virtual void getGameRequests                ( const std::string& app_id ) {}
    std::set<std::string>& fetchGameRequests    ( );
            void clearGameRequests              ( );

    		void addDelegate              		( FacebookDelegate* delegate );
    		void removeDelegate            		( FacebookDelegate* delegate );

    virtual bool isLoggedIn                     ( ) const;
    virtual void gameRequestsCallback           ( bool success,
                                                  const std::map<std::string, std::shared_ptr<FacebookGameRequest>>& req );
    
    void appInviteComplete                      ( bool success );
    void gameRequestComplete                    ( bool success );
    void shareComplete                          ( bool success );

protected:
    virtual void userLoggedIn                   ( bool success );
    virtual void userLoggedOut                  ( );
    virtual void userPosted                     ( );
    virtual void errorCallback                  ( long error );
    virtual void userDataCallback               ( const std::map<std::string,std::string>& me_data );
    virtual void friendDataCallback             ( const std::vector<std::map<std::string,std::string>>& friends_data );
    virtual void gameRequestDeletedCallback     ( bool success, const std::shared_ptr<FacebookGameRequest>& req );
    virtual void gameRequestRecievedCallback    ( const std::set<std::string>& reqs );

    Facebook();

    std::vector<FacebookDelegate*> _delegates;
    std::set<std::string>          _game_reqs;
    
};

inline void Facebook::addDelegate (FacebookDelegate* delegate) { _delegates.push_back(delegate); }
inline bool Facebook::isLoggedIn()  const { return false; }
inline std::set<std::string>& Facebook::fetchGameRequests() { return _game_reqs; }
inline void Facebook::clearGameRequests() { _game_reqs.clear(); }

#endif /* defined(__Pirateer__Facebook__) */
