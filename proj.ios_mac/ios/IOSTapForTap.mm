
#include <../Classes/external/TapForTap.h>
#import "TapForTap/TFTTapForTap.h"

#import "AppController.h"
#import "RootViewController.h"

@interface TapForTapController : NSObject <TFTInterstitialDelegate> {
}
@end

@implementation TapForTapController

- (void)tftInterstitialDidReceiveAd:(TFTInterstitial *)interstitial
{
    TapForTapInterstitialDelegate* delegate = TapForTap::sharedTFT()->getDelegate();
    if (delegate) delegate->InterstitialDidReceiveAd();
}
- (void)tftInterstitial:(TFTInterstitial *)interstitial didFail:(NSString *)reason
{
    TapForTapInterstitialDelegate* delegate = TapForTap::sharedTFT()->getDelegate();
    if (delegate) delegate->Interstitial();

}
- (void)tftInterstitialDidShow:(TFTInterstitial *)interstitial
{
    TapForTapInterstitialDelegate* delegate = TapForTap::sharedTFT()->getDelegate();
    if (delegate) delegate->InterstitialDidShow();

}
- (void)tftInterstitialWasTapped:(TFTInterstitial *)interstitial
{
    TapForTapInterstitialDelegate* delegate = TapForTap::sharedTFT()->getDelegate();
    if (delegate) delegate->InterstitialWasTapped();

}
- (void)tftInterstitialWasDismissed:(TFTInterstitial *)interstitial
{
    TapForTapInterstitialDelegate* delegate = TapForTap::sharedTFT()->getDelegate();
    if (delegate) delegate->InterstitialWasDismissed();

}

@end

TFTInterstitial* s_TFTInterstitial = nil;

TapForTapController* s_TFTDelegate = nil;

TapForTap::TapForTap()
{
}

void TapForTap::purge()
{
    if (s_TFTDelegate)  [s_TFTDelegate dealloc];
    if (s_TFTInterstitial) [s_TFTInterstitial release];
    s_TFTDelegate = nullptr;
    s_TFTInterstitial = nullptr;
}

bool TapForTap::init()
{
    if (!s_TFTDelegate) s_TFTDelegate = [TapForTapController alloc];
    s_TFTInterstitial = [[TFTInterstitial interstitialWithDelegate:s_TFTDelegate] retain];
    //[TFTInterstitial load];
    return true;
}

void TapForTap::hideAds()
{
    /*AppController* appController = (AppController*) [UIApplication sharedApplication].delegate;
    for (UIView* subView in appController.viewController.view.subviews) {
        if (static_cast<TFTInterstitial*>(subView) != NULL) {
            [subView removeFromSuperview];
        }
    }
    [TapForTapInterstitial close];*/
}

bool TapForTap::hasInterstitial()
{
    return [s_TFTInterstitial readyToShow];
}

void TapForTap::cacheInterstitial()
{
    return [s_TFTInterstitial load];
}

void TapForTap::showInterstitial()
{
    //[TapForTapInterstitial prepare];

    AppController* appController = (AppController*) [UIApplication sharedApplication].delegate;
    [s_TFTInterstitial showWithViewController:[appController viewController]];
}

void TapForTap::showBanner()
{
    // TapForTap stores banner ads in 320x50 and 640x100 (retina) and the SDK
    // selects the ad that fits in the size we give to initWithFrame. So we
    // simply detect if we're wide enough for the 100px width banner.
    /*CGRect screenRect = [[UIScreen mainScreen] bounds];
    CGFloat adHeight = (screenRect.size.width < 640) ? 50 : 100;
    CGFloat posY = screenRect.size.height - adHeight;
    
    TapForTapAdView* adView = [[TapForTapAdView alloc] initWithFrame:CGRectMake(0, posY, screenRect.size.width, adHeight)];
    AppController* appController = (AppController*) [UIApplication sharedApplication].delegate;
    [appController->viewController.view addSubview:adView];
    [adView release];*/
}
