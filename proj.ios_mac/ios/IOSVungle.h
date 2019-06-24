//
//  IOSVungle.h
//  Pirateer
//
//  Created by Peter Respondek on 5/23/14.
//
//


#ifndef __Pirateer__IOSVungle__
#define __Pirateer__IOSVungle__

#include "Vungle.h"

class IOSVungle : public Vungle {
public:
    virtual ~IOSVungle();
    CREATE_FUNC(IOSVungle);
    bool init();
    virtual void playAd(VungleDelegate* delegate = nullptr);
    virtual void vungleSDKwillShowAd();
    virtual void vungleSDKwillCloseAdWithViewInfo(bool completedView, int playTime, bool didDownload, bool willPresentProductSheet);
    virtual void vungleSDKwillCloseProductSheet();
    virtual bool isCachedAdAvailable();
    virtual void vungleSDKhasCachedAdAvailable();
    
    void* vc;
};

#endif /* defined(__Pirateer__IOSVungle__) */
