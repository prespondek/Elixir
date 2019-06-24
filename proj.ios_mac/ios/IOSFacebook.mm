//
//  IOSFacebook.cpp
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#include "IOSFacebook.h"
#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>
#import <FBSDKShareKit/FBSDKShareKit.h>
#import "AppController.h"


@interface FacebookIOS : NSObject<FBSDKAppInviteDialogDelegate, FBSDKGameRequestDialogDelegate, FBSDKSharingDelegate>
{
}

+ (FacebookIOS*)shared;

- (void)gameRequestDialog:(FBSDKGameRequestDialog *)gameRequestDialog didCompleteWithResults:(NSDictionary *)results;

- (void)gameRequestDialog:(FBSDKGameRequestDialog *)gameRequestDialog didFailWithError:(NSError *)error;

- (void)gameRequestDialogDidCancel:(FBSDKGameRequestDialog *)gameRequestDialog;

- (void)appInviteDialog:(FBSDKAppInviteDialog *)appInviteDialog didCompleteWithResults:(NSDictionary *)results;

- (void)appInviteDialog:(FBSDKAppInviteDialog *)appInviteDialog didFailWithError:(NSError *)error;


@end

static FacebookIOS* instance = nil;

@implementation FacebookIOS

+ (FacebookIOS*)shared
{
    @synchronized(self) {
        if (instance == nil) {
            instance = [[self alloc] init];
        }
    }
    return instance;
}

- (void)gameRequestDialog:(FBSDKGameRequestDialog *)gameRequestDialog didCompleteWithResults:(NSDictionary *)results
{
    Facebook::getInstance()->gameRequestComplete(true);
}

- (void)gameRequestDialog:(FBSDKGameRequestDialog *)gameRequestDialog didFailWithError:(NSError *)error
{
    Facebook::getInstance()->gameRequestComplete(false);
    if (error.code == 100) {
        Facebook::getInstance()->logIn();
    }
}

- (void)gameRequestDialogDidCancel:(FBSDKGameRequestDialog *)gameRequestDialog
{
    Facebook::getInstance()->gameRequestComplete(false);
}

- (void)appInviteDialog:(FBSDKAppInviteDialog *)appInviteDialog didCompleteWithResults:(NSDictionary *)results
{
    Facebook::getInstance()->appInviteComplete(true);
}

- (void)appInviteDialog:(FBSDKAppInviteDialog *)appInviteDialog didFailWithError:(NSError *)error
{
    Facebook::getInstance()->appInviteComplete(false);
    if (error.code == 100) {
        Facebook::getInstance()->logIn();
    }
}

- (void)sharer:(id<FBSDKSharing>)sharer didCompleteWithResults:(NSDictionary *)results { 
    Facebook::getInstance()->shareComplete (true);
}

- (void)sharer:(id<FBSDKSharing>)sharer didFailWithError:(NSError *)error { 
    Facebook::getInstance()->shareComplete (false);
}

- (void)sharerDidCancel:(id<FBSDKSharing>)sharer { 
    Facebook::getInstance()->shareComplete (false);
}

@end


void IOSFacebook::performPublishAction() {

}

void IOSFacebook::publishActionComplete()
{
    
}

IOSFacebook::~IOSFacebook()
{
}

bool IOSFacebook::init()
{
    [FBSDKProfile enableUpdatesOnAccessTokenChange:YES];
    return true;
}

void IOSFacebook::openAppInviteDialog( const std::string& app_link, const std::string& img_url )
{
    FBSDKAppInviteContent *content =[[FBSDKAppInviteContent alloc] init];
    content.appLinkURL = [NSURL URLWithString:[NSString stringWithUTF8String:app_link.c_str()]];
    content.appInvitePreviewImageURL = [NSURL URLWithString:[NSString stringWithUTF8String:img_url.c_str()]];
    [FBSDKAppInviteDialog showWithContent:content delegate:[FacebookIOS shared]];
}




void IOSFacebook::getGameRequests( const std::string& app_id )
{
    FBSDKGraphRequest* request = [[FBSDKGraphRequest alloc] initWithGraphPath:@"me/apprequests" parameters:nil];
    [request setGraphErrorRecoveryDisabled:YES];
    [request startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
         
         // to avoid a lot of redudant game requests I stick the game request into
         std::map<std::string, std::shared_ptr<FacebookGameRequest>> reqs;
         if (!error) {
             NSArray* data = ((NSDictionary*)result) [@"data"];
             for ( NSDictionary* req in data ) {
                 NSString* req_app_id = req[@"application"][@"id"];
                 if (app_id.compare(req_app_id.UTF8String) != 0) continue;
                 NSString* req_id =     req[@"id"];
                 NSString* message =    req[@"message"];
                 NSString* req_data =   req[@"data"];
                 NSString* from_name =  req[@"from"][@"name"];
                 NSString* from_id =    req[@"from"][@"id"];
                 if (!req_data) {
                     req_data =@"0";
                 }
                 std::string key = std::string(req_data.UTF8String) + std::string(from_id.UTF8String);
                 auto iter = reqs.find(key);
                 if (iter == reqs.end()) {
                     FacebookGameRequest* game_req =    new FacebookGameRequest();
                     game_req->message =                message.UTF8String;
                     game_req->from_id =                from_id.UTF8String;
                     game_req->type =                   (FBGameRequestType)req_data.integerValue;
                     game_req->from_name =              from_name.UTF8String;
                     game_req->req_ids.push_back(req_id.UTF8String);
                     reqs.insert( std::pair<std::string,
                                  std::shared_ptr<FacebookGameRequest>>(key,
                                  std::shared_ptr<FacebookGameRequest>(game_req)));
                 } else {
                     iter->second->req_ids.push_back(req_id.UTF8String);
                 }
             }
         } else {
             errorCallback(error.code);
         }
         gameRequestsCallback(!error, reqs);
     }];
}

void IOSFacebook::getFriendData( )
{

    FBSDKGraphRequest* request = [[FBSDKGraphRequest alloc] initWithGraphPath:@"me/friends" parameters:@{@"fields" : @"name,id,picture.width(200).height(200)"}];

    [request setGraphErrorRecoveryDisabled:YES];
    [request startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection,
                                          id result,
                                          NSError *error) {
        std::vector<std::map<std::string,std::string>> friends_data;
        if (error) {
            errorCallback(error.code);
            friendDataCallback(friends_data);
            return;
        }
        NSDictionary* dict = (NSDictionary*)result;

        NSArray* friends = (NSArray*)dict[@"data"];
        if ( friends.count == 0 ) {
            friendDataCallback(friends_data);
            return;
        }
        //FBSDKGraphRequestConnection *batch = [[FBSDKGraphRequestConnection alloc] init];

        for (NSDictionary* friend_dict in friends ) {
            std::map<std::string, std::string> friend_data;
            //NSMutableString* permissions = [NSMutableString string];
            NSString* friend_name =   friend_dict[@"name"];
            NSString* friend_id =     friend_dict[@"id"];
            /*FBSDKGraphRequest *friend_permissions = [[FBSDKGraphRequest alloc]
                                               initWithGraphPath:[NSString stringWithFormat:@"/%@/permissions",friend_id]
                                                     parameters:nil];
            [batch addRequest:friend_permissions
                 completionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
                     NSLog(@"%@", error.description);
                     NSDictionary* dict = (NSDictionary*)result;
                     NSArray* pers = (NSArray*)dict[@"permissions"][@"data"];
                     for ( NSDictionary* key in pers ) {
                         if ( [(NSString*)key[@"status"] isEqualToString:@"granted"] ) {
                             [permissions appendString:[NSString stringWithFormat:@"%@;",key[@"permission"]]];
                         }
                     }
                 }];*/
            friend_data.insert(std::pair<std::string,std::string>("name", friend_name.UTF8String));
            friend_data.insert(std::pair<std::string,std::string>("id", friend_id.UTF8String));
            //friend_data.insert(std::pair<std::string,std::string>("permissions", permissions.UTF8String));


            NSDictionary* friend_pic = (NSDictionary*)friend_dict[@"picture"];
            NSString* friend_url =         friend_pic[@"data"][@"url"];
            NSNumber* friend_silhouette =  friend_pic[@"data"][@"is_silhouette"];
            if ( friend_silhouette.boolValue == TRUE ) {
                friend_url = @"";
            }
            friend_data.insert(std::pair<std::string,std::string>("picture",friend_url.UTF8String));
            friends_data.push_back(friend_data);
        }
        ///[batch start];
        friendDataCallback(friends_data);
    }];
    
}

void IOSFacebook::refreshAccessToken()
{
    [FBSDKAccessToken refreshCurrentAccessToken:^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
        if (error) {
            NSLog(@"Process error");
            errorCallback(error.code);
            userLoggedIn(false);
        } else {
            NSLog(@"Logged in");
            userLoggedIn(true);
        }
    }];
}

void IOSFacebook::getUserData()
{
    FBSDKGraphRequest* request = [[FBSDKGraphRequest alloc] initWithGraphPath:@"me" parameters:@{@"fields" : @"permissions,name,email,picture.width(200).height(200),id"}];
    [request setGraphErrorRecoveryDisabled:YES];
    [request startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
        std::map<std::string,std::string> user_data;
        if (error) {
            errorCallback(error.code);
            userDataCallback(user_data);
            return;
        }
        NSDictionary* dict = (NSDictionary*)result;
        
        NSMutableString* permissions = [NSMutableString string];
        NSString* user_name =   dict[@"name"];
        NSString* user_id =     dict[@"id"];
        NSString* user_email =  dict[@"email"];
        NSArray* pers = (NSArray*)dict[@"permissions"][@"data"];
        for ( NSDictionary* key in pers ) {
            if ( [(NSString*)key[@"status"] isEqualToString:@"granted"] ) {
                [permissions appendString:[NSString stringWithFormat:@"%@;",key[@"permission"]]];
            }
        }

        user_data.insert(std::pair<std::string,std::string>("name", user_name.UTF8String));
        user_data.insert(std::pair<std::string,std::string>("id", user_id.UTF8String));
        user_data.insert(std::pair<std::string,std::string>("email", user_email.UTF8String));
        user_data.insert(std::pair<std::string,std::string>("permissions", permissions.UTF8String));

        NSDictionary* pic = (NSDictionary*)dict[@"picture"];
        NSString* url =         pic[@"data"][@"url"];
        NSNumber* silhouette =  pic[@"data"][@"is_silhouette"];
        if (url && silhouette.boolValue == FALSE) {
            user_data.insert(std::pair<std::string,std::string>("picture",url.UTF8String));
        }
        userDataCallback(user_data);
    }];
}

void IOSFacebook::sendGameRequest( FBGameRequestType type,
                                   const std::string& message,
                                   const std::string& title, const
                                   std::string& recipient )
{
    if (!isLoggedIn()) return;
    FBSDKGameRequestContent *gameRequestContent = [[FBSDKGameRequestContent alloc] init];
    gameRequestContent.message = [NSString stringWithUTF8String:message.c_str()];
    gameRequestContent.title = [NSString stringWithUTF8String:title.c_str()];
    gameRequestContent.data = [NSString stringWithFormat:@"%d",(uint8_t)type];
    if (!recipient.empty()) {
        gameRequestContent.recipients = @[[NSString stringWithUTF8String:recipient.c_str()]];
    }

    [FBSDKGameRequestDialog showWithContent:gameRequestContent delegate:[FacebookIOS shared]];
}

void IOSFacebook::deleteGameRequest      ( const std::shared_ptr<FacebookGameRequest>& reqs )
{
    if (reqs->req_ids.size() == 0) {
        return;
    }
    std::shared_ptr<FacebookGameRequest> greq = reqs;
    __block size_t counter = greq->req_ids.size();
    __block bool success = false;
    FBSDKGraphRequestConnection *connection = [[FBSDKGraphRequestConnection alloc] init];

    for ( auto& req : greq->req_ids) {
        FBSDKGraphRequest *request = [[FBSDKGraphRequest alloc] initWithGraphPath:[NSString stringWithUTF8String:req.c_str()] parameters:nil HTTPMethod:@"DELETE"];
        [request setGraphErrorRecoveryDisabled:YES];
        [connection addRequest:request completionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
            CCASSERT(counter, "counter overrun.");
            if (!error) {
                success = true;
            } else {
                errorCallback(error.code);
            }
            counter = counter - 1;
            if (counter == 0) {
                gameRequestDeletedCallback( success, greq );
            }
        }];
    }
    [connection start];
}

bool IOSFacebook::isLoggedIn( ) const
{
    if ([FBSDKAccessToken currentAccessToken]) {
        return true;
    }
    return false;
}

void IOSFacebook::logOut( )
{
    FBSDKLoginManager *loginManager = [[FBSDKLoginManager alloc] init];
    [loginManager logOut];
    [FBSDKAccessToken setCurrentAccessToken:nil];
    
}

void IOSFacebook::logIn( )
{
    [FBSDKAccessToken setCurrentAccessToken:nil];
    FBSDKLoginManager *login = [[FBSDKLoginManager alloc] init];
    UIViewController* root_view_controller = [UIApplication sharedApplication].keyWindow.rootViewController;
    [login
     logInWithReadPermissions: @[@"public_profile", @"email", @"user_friends"]
     fromViewController: root_view_controller
     handler:^(FBSDKLoginManagerLoginResult *result, NSError *error) {
         if (error) {
             NSLog(@"Process error");
             errorCallback(error.code);
             userLoggedIn(false);
         } else if (result.isCancelled) {
             NSLog(@"Cancelled");
             userLoggedIn(false);
         } else {
             NSLog(@"Logged in");
             userLoggedIn(true);
         }
     }];
}

void IOSFacebook::postStatusUpdate( const std::string& caption )
{
    
}

void IOSFacebook::postLink(const std::string &link)
{
    FBSDKShareLinkContent *content = [[FBSDKShareLinkContent alloc] init];
    content.contentURL = [NSURL URLWithString:[NSString stringWithUTF8String:link.c_str()]];
    [FBSDKShareDialog showFromViewController:[UIApplication sharedApplication].keyWindow.rootViewController
                                 withContent:content
                                    delegate:[FacebookIOS shared]];
}
