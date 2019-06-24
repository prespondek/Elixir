//
//  CCIOSReachability.cpp
//  Pirateer
//
//  Created by Peter Respondek on 6/24/14.
//
//

#include "CCIOSReachability.h"
#import "IOSReachability.h"

@interface IOSReachabilityBridge : NSObject <IOSReachabilityDelegate>
@end

static IOSReachability* g_hostReachability = nil;
static IOSReachability* g_internetReachability = nil;
static IOSReachability* g_wifiReachability = nil;
static IOSReachabilityBridge* g_ReachAbilityDelegate = nil;

CCIOSReachability::~CCIOSReachability() {
    if (g_ReachAbilityDelegate) {
        [g_ReachAbilityDelegate dealloc];
    }
}

bool CCIOSReachability::init()
{
    if (!g_ReachAbilityDelegate) {
        g_ReachAbilityDelegate = [[IOSReachabilityBridge alloc] init];
    }
    return true;
}

bool CCIOSReachability::checkReachabilityForInternetConnection() {
    IOSReachability* internetReachability = [IOSReachability reachabilityForInternetConnection];
    return [internetReachability currentReachabilityStatus];
}

void CCIOSReachability::monitorReachabilityWithHostName( const char* host_name ) {
    if (!g_hostReachability) {
        g_hostReachability = [IOSReachability reachabilityWithHostName:[NSString stringWithCString:host_name encoding:NSASCIIStringEncoding]];
    }
    [g_hostReachability startNotifier];
    [g_hostReachability setDelegate:g_ReachAbilityDelegate];
}

@implementation IOSReachabilityBridge

-(void)reachabilityStatusChanged:(NetworkStatus) status
{
    CCReachability::sharedReachability()->reachabilityStatusChanged( (ReachabilityStatus)status );
}

@end

/*

@interface IOSReachabilityBridge : NSObject

@property (nonatomic, strong) IOSReachability *hostReachability;
@property (nonatomic, strong) IOSReachability *internetReachability;
@property (nonatomic, strong) IOSReachability *wifiReachability;

@end

@implementation IOSReachabilityBridge

-(void) monitorInternetConnection
{
    [self.internetReachability startNotifier];
}

-(void) monitorLocalWifi
{
    [self.wifiReachability startNotifier];
}

-(void) monitorHost
{
    [self.hostReachability startNotifier];
}

-(id) init
{
    self.hostReachability = [IOSReachability reachabilityWithHostName:remoteHostName];
    self.internetReachability = [IOSReachability reachabilityForInternetConnection];
    self.wifiReachability = [IOSReachability reachabilityForLocalWiFi];
	
}

@end
*/


