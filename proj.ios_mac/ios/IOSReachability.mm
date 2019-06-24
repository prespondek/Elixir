/*
     File: Reachability.m
 Abstract: Basic demonstration of how to use the SystemConfiguration Reachablity APIs.
  Version: 3.5
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
 */

#import <arpa/inet.h>
//#import <ifaddrs.h>
//#import <netdb.h>
//#import <sys/socket.h>

//#import <CoreFoundation/CoreFoundation.h>

#include "IOSReachability.h"

//#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
//#import <netinet/in.h>


typedef enum : NSInteger {
    NotReachable = 0,
    ReachableViaWiFi,
    ReachableViaWWAN
} NetworkStatus;


extern NSString *kReachabilityChangedNotification;

@protocol IOSReachabilityDelegate <NSObject>

-(void)reachabilityStatusChanged:(NetworkStatus) status;

@end


@interface ReachabilityIOS : NSObject

/*!
 * Use to check the reachability of a given host name.
 */
+ (instancetype)reachabilityWithHostName:(NSString *)hostName;

/*!
 * Use to check the reachability of a given IP address.
 */
+ (instancetype)reachabilityWithAddress:(const struct sockaddr_in *)hostAddress;

/*!
 * Checks whether the default route is available. Should be used by applications that do not connect to a particular host.
 */
+ (instancetype)reachabilityForInternetConnection;

/*!
 * Checks whether a local WiFi connection is available.
 */
+ (instancetype)reachabilityForLocalWiFi;

/*!
 * Start listening for reachability notifications on the current run loop.
 */
- (BOOL)startNotifier;
- (void)stopNotifier;

- (NetworkStatus)currentReachabilityStatus;

/*!
 * WWAN may be available, but not active until a connection has been established. WiFi may require a connection for VPN on Demand.
 */
- (BOOL)connectionRequired;

@property (nonatomic,retain) id delegate;

@end


NSString *kReachabilityChangedNotification = @"kNetworkReachabilityChangedNotification";


#pragma mark - Supporting functions

#define kShouldPrintReachabilityFlags 1

static void PrintReachabilityFlags(SCNetworkReachabilityFlags flags, const char* comment)
{
#if kShouldPrintReachabilityFlags

    NSLog(@"Reachability Flag Status: %c%c %c%c%c%c%c%c%c %s\n",
          (flags & kSCNetworkReachabilityFlagsIsWWAN)				? 'W' : '-',
          (flags & kSCNetworkReachabilityFlagsReachable)            ? 'R' : '-',

          (flags & kSCNetworkReachabilityFlagsTransientConnection)  ? 't' : '-',
          (flags & kSCNetworkReachabilityFlagsConnectionRequired)   ? 'c' : '-',
          (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic)  ? 'C' : '-',
          (flags & kSCNetworkReachabilityFlagsInterventionRequired) ? 'i' : '-',
          (flags & kSCNetworkReachabilityFlagsConnectionOnDemand)   ? 'D' : '-',
          (flags & kSCNetworkReachabilityFlagsIsLocalAddress)       ? 'l' : '-',
          (flags & kSCNetworkReachabilityFlagsIsDirect)             ? 'd' : '-',
          comment
          );
#endif
}


static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info)
{
#pragma unused (target, flags)
	NSCAssert(info != NULL, @"info was NULL in ReachabilityCallback");
	NSCAssert([(__bridge NSObject*) info isKindOfClass: [ReachabilityIOS class]], @"info was wrong class in ReachabilityCallback");

    IOSReachability* noteObject = (__bridge IOSReachability *)info;
    // Post a notification to notify the client that the network reachability changed.
    //[[NSNotificationCenter defaultCenter] postNotificationName: kReachabilityChangedNotification object: noteObject];
    [[noteObject delegate]reachabilityStatusChanged:[noteObject currentReachabilityStatus]];
}


#pragma mark - Reachability implementation

@implementation ReachabilityIOS
{
	BOOL _alwaysReturnLocalWiFiStatus; //default is NO
	SCNetworkReachabilityRef _reachabilityRef;
}

+ (instancetype)reachabilityWithHostName:(NSString *)hostName
{
	ReachabilityIOS* returnValue = NULL;
	SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(NULL, [hostName UTF8String]);
	if (reachability != NULL)
	{
		returnValue= [[self alloc] init];
		if (returnValue != NULL)
		{
			returnValue->_reachabilityRef = reachability;
			returnValue->_alwaysReturnLocalWiFiStatus = NO;
		}
	}
	return returnValue;
}


+ (instancetype)reachabilityWithAddress:(const struct sockaddr_in *)hostAddress
{
	SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr *)hostAddress);

	ReachabilityIOS* returnValue = NULL;

	if (reachability != NULL)
	{
		returnValue = [[self alloc] init];
		if (returnValue != NULL)
		{
			returnValue->_reachabilityRef = reachability;
			returnValue->_alwaysReturnLocalWiFiStatus = NO;
		}
	}
	return returnValue;
}



+ (instancetype)reachabilityForInternetConnection
{
	struct sockaddr_in zeroAddress;
	bzero(&zeroAddress, sizeof(zeroAddress));
	zeroAddress.sin_len = sizeof(zeroAddress);
	zeroAddress.sin_family = AF_INET;
    
	return [self reachabilityWithAddress:&zeroAddress];
}


+ (instancetype)reachabilityForLocalWiFi
{
	struct sockaddr_in localWifiAddress;
	bzero(&localWifiAddress, sizeof(localWifiAddress));
	localWifiAddress.sin_len = sizeof(localWifiAddress);
	localWifiAddress.sin_family = AF_INET;

	// IN_LINKLOCALNETNUM is defined in <netinet/in.h> as 169.254.0.0.
	localWifiAddress.sin_addr.s_addr = htonl(IN_LINKLOCALNETNUM);

	ReachabilityIOS* returnValue = [self reachabilityWithAddress: &localWifiAddress];
	if (returnValue != NULL)
	{
		returnValue->_alwaysReturnLocalWiFiStatus = YES;
	}
    
	return returnValue;
}


#pragma mark - Start and stop notifier

- (BOOL)startNotifier
{
	BOOL returnValue = NO;
	SCNetworkReachabilityContext context = {0, (__bridge void *)(self), NULL, NULL, NULL};

	if (SCNetworkReachabilitySetCallback(_reachabilityRef, ReachabilityCallback, &context))
	{
		if (SCNetworkReachabilityScheduleWithRunLoop(_reachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode))
		{
			returnValue = YES;
		}
	}
    
	return returnValue;
}


- (void)stopNotifier
{
	if (_reachabilityRef != NULL)
	{
		SCNetworkReachabilityUnscheduleFromRunLoop(_reachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}
}


- (void)dealloc
{
	[self stopNotifier];
	if (_reachabilityRef != NULL)
	{
		CFRelease(_reachabilityRef);
	}
    [super dealloc];
}


#pragma mark - Network Flag Handling

- (NetworkStatus)localWiFiStatusForFlags:(SCNetworkReachabilityFlags)flags
{
	PrintReachabilityFlags(flags, "localWiFiStatusForFlags");
	NetworkStatus returnValue = NotReachable;

	if ((flags & kSCNetworkReachabilityFlagsReachable) && (flags & kSCNetworkReachabilityFlagsIsDirect))
	{
		returnValue = ReachableViaWiFi;
	}
    
	return returnValue;
}


- (NetworkStatus)networkStatusForFlags:(SCNetworkReachabilityFlags)flags
{
	PrintReachabilityFlags(flags, "networkStatusForFlags");
	if ((flags & kSCNetworkReachabilityFlagsReachable) == 0)
	{
		// The target host is not reachable.
		return NotReachable;
	}

    NetworkStatus returnValue = NotReachable;

	if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0)
	{
		/*
         If the target host is reachable and no connection is required then we'll assume (for now) that you're on Wi-Fi...
         */
		returnValue = ReachableViaWiFi;
	}

	if ((((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
        (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0))
	{
        /*
         ... and the connection is on-demand (or on-traffic) if the calling application is using the CFSocketStream or higher APIs...
         */

        if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0)
        {
            /*
             ... and no [user] intervention is needed...
             */
            returnValue = ReachableViaWiFi;
        }
    }

	if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN)
	{
		/*
         ... but WWAN connections are OK if the calling application is using the CFNetwork APIs.
         */
		returnValue = ReachableViaWWAN;
	}
    
	return returnValue;
}


- (BOOL)connectionRequired
{
	NSAssert(_reachabilityRef != NULL, @"connectionRequired called with NULL reachabilityRef");
	SCNetworkReachabilityFlags flags;

	if (SCNetworkReachabilityGetFlags(_reachabilityRef, &flags))
	{
		return (flags & kSCNetworkReachabilityFlagsConnectionRequired);
	}

    return NO;
}


- (NetworkStatus)currentReachabilityStatus
{
	NSAssert(_reachabilityRef != NULL, @"currentNetworkStatus called with NULL SCNetworkReachabilityRef");
	NetworkStatus returnValue = NotReachable;
	SCNetworkReachabilityFlags flags;
    
	if (SCNetworkReachabilityGetFlags(_reachabilityRef, &flags))
	{
		if (_alwaysReturnLocalWiFiStatus)
		{
			returnValue = [self localWiFiStatusForFlags:flags];
		}
		else
		{
			returnValue = [self networkStatusForFlags:flags];
		}
	}
    
	return returnValue;
}


@end

@interface IOSReachabilityBridge : NSObject <IOSReachabilityDelegate>
@end

static ReachabilityIOS* g_hostReachability = nil;
//static ReachabilityIOS* g_internetReachability = nil;
//static ReachabilityIOS* g_wifiReachability = nil;
static IOSReachabilityBridge* g_ReachAbilityDelegate = nil;

IOSReachability::~IOSReachability() {
    if (g_ReachAbilityDelegate) {
        [g_ReachAbilityDelegate dealloc];
    }
}

bool IOSReachability::init()
{
    if (!g_ReachAbilityDelegate) {
        g_ReachAbilityDelegate = [[IOSReachabilityBridge alloc] init];
    }
    return true;
}

bool IOSReachability::checkReachabilityForInternetConnection() {
    ReachabilityIOS* internetReachability = [ReachabilityIOS reachabilityForInternetConnection];
    return [internetReachability currentReachabilityStatus];
}

void IOSReachability::monitorReachabilityWithHostName( const char* host_name ) {
    if (!g_hostReachability) {
        g_hostReachability = [ReachabilityIOS reachabilityWithHostName:[NSString stringWithCString:host_name encoding:NSASCIIStringEncoding]];
    }
    [g_hostReachability startNotifier];
    [g_hostReachability setDelegate:g_ReachAbilityDelegate];
}

@implementation IOSReachabilityBridge

-(void)reachabilityStatusChanged:(NetworkStatus) status
{
    Reachability::getInstance()->reachabilityStatusChanged( (ReachabilityStatus)status );
}

@end
