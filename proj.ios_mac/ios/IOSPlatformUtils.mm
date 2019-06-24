//
//  IOSFacebook.cpp
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#include "IOSPlatformUtils.h"
#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>
#import "AppController.h"


void IOSPlatformUtils::downloadUrl( const std::string& url, const std::string& path, const std::function<void (const std::string&, const std::string&, bool)>& func)
{
    NSURL *pictureURL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
    NSURLRequest *urlRequest = [NSURLRequest requestWithURL:pictureURL];
    
    std::string c_url = url; std::string c_path = path;
    std::function<void (const std::string&, const std::string&, bool)> c_func = func;
    //userImageCallback(url.UTF8String, false);
    [NSURLConnection sendAsynchronousRequest:urlRequest
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *connectionError)
    {
         if ( connectionError == nil && data != nil ) {
             NSError* error;
             BOOL success = [data writeToFile:[NSString stringWithUTF8String:c_path.c_str()]options:NSDataWritingAtomic error:&error];
             if (success) {
                 c_func(c_url,c_path,true);
                 return;
             }
         }
        c_func(c_url,c_path,false);
    }];
}

void IOSPlatformUtils::getBuildNumber ( std::string& out_version, std::string& out_build )
{
    out_version = [[[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleShortVersionString"] UTF8String];
    out_build = [[[NSBundle mainBundle] objectForInfoDictionaryKey: (NSString *)kCFBundleVersionKey] UTF8String];
}


bool IOSPlatformUtils::init()
{
    return true;
}

void IOSPlatformUtils::createUUID(std::string &uuid)
{
    NSString* new_uuid = [[NSUUID UUID] UUIDString];
    uuid = new_uuid.UTF8String;
}
