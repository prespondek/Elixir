//
//  IOSVungle.cpp
//  Pirateer
//
//  Created by Peter Respondek on 5/23/14.
//
//

#import <VungleSDK/VungleSDK.h>
#include "IOSVungle.h"



@interface VungleController : NSObject <VungleSDKDelegate, VungleSDKLogger> {
    IOSVungle* controller;
}
- (id)init: (IOSVungle*) parent;

@end

bool IOSVungle::init()
{
    vc = [[VungleController alloc]init:(this)];
    [[VungleSDK sharedSDK] setDelegate:(VungleController*)vc];
#if defined (VUNGLE_DEBUG) && (VUNGLE_DEBUG > 0)
    [[VungleSDK sharedSDK] setLoggingEnabled:(TRUE)];
    [[VungleSDK sharedSDK] attachLogger:(VungleController*)vc];
#endif
    if (vc) return true;
    return false;
}
IOSVungle::~IOSVungle()
{
    [[VungleSDK sharedSDK] setDelegate:nil];
    [(VungleController*)vc release];
}

void IOSVungle::playAd(VungleDelegate* delegate)
{
    Vungle::playAd(delegate);
    UIWindow *window = [[[UIApplication sharedApplication] delegate] window];
    VungleSDK* sdk = [VungleSDK sharedSDK];
    [sdk playAd:window.rootViewController];
    
}

bool IOSVungle::isCachedAdAvailable()
{
    return [[VungleSDK sharedSDK] isCachedAdAvailable];
}


void IOSVungle::vungleSDKwillShowAd()
{
    Vungle::vungleSDKwillShowAd();
}
void IOSVungle::vungleSDKwillCloseAdWithViewInfo(bool completedView, int playTime, bool didDownload, bool willPresentProductSheet)
{
    Vungle::vungleSDKwillCloseAdWithViewInfo(completedView, playTime, didDownload, willPresentProductSheet);
}
void IOSVungle::vungleSDKwillCloseProductSheet()
{
    Vungle::vungleSDKwillCloseProductSheet();
}

void IOSVungle::vungleSDKhasCachedAdAvailable()
{
    Vungle::vungleSDKhasCachedAdAvailable();
}

@implementation VungleController

- (id) init:(IOSVungle *)parent {
    if (self = [super init]) {
        controller = parent;
    }
    return self;
}
/**
 * if implemented, this will get called when the SDK is about to show an ad. This point
 * might be a good time to pause your game, and turn off any sound you might be playing.
 */
- (void)vungleSDKwillShowAd {
    controller->vungleSDKwillShowAd();
}

- (void)vungleSDKLog:(NSString*)message
{
    
}

/**
 * if implemented, this will get called when the SDK closes the ad view, but there might be
 * a product sheet that will be presented. This point might be a good place to resume your game
 * if there's no product sheet being presented. The viewInfo dictionary will contain the
 * following keys:
 * - "completedView": NSNumber representing a BOOL whether or not the video can be considered a
 *               full view.
 * - "playTime": NSNumber representing the time in seconds that the user watched the video.
 * - "didDownlaod": NSNumber representing a BOOL whether or not the user clicked the download
 *                  button.
 */
- (void)vungleSDKwillCloseAdWithViewInfo:(NSDictionary*)viewInfo willPresentProductSheet:(BOOL)willPresentProductSheet {
    NSNumber* comp = [viewInfo objectForKey:@"completedView"];
    NSNumber* playt = [viewInfo objectForKey:@"playTime"];
    NSNumber* downl = [viewInfo objectForKey:@"didDownload"];
    
    controller->vungleSDKwillCloseAdWithViewInfo( comp.boolValue, playt.integerValue , downl.boolValue, willPresentProductSheet );
}

/**
 * if implemented, this will get called when the product sheet is about to be closed.
 */
- (void)vungleSDKwillCloseProductSheet:(id)productSheet {
    controller->vungleSDKwillCloseProductSheet();
}

- (void)vungleSDKhasCachedAdAvailable {
    controller->vungleSDKhasCachedAdAvailable();
}


@end


