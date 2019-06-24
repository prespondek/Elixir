//
//  Facebook.cpp
//  Pirateer
//
//  Created by Peter Respondek on 7/15/14.
//
//

#include "../external/PlatformUtils.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "../../proj.ios_mac/ios/IOSPlatformUtils.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../../proj.android/app/jni/AndroidPlatformUtils.h"

#endif

static PlatformUtils* s_PlatformUtils = nullptr;

PlatformUtils::PlatformUtils()
{
}

PlatformUtils::~PlatformUtils()
{
}

PlatformUtils* PlatformUtils::getInstance() {
    if (!s_PlatformUtils) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        s_PlatformUtils = new IOSPlatformUtils();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        s_PlatformUtils = new AndroidPlatformUtils();
#endif
        s_PlatformUtils->init();
    }
    return s_PlatformUtils;
}

void PlatformUtils::purgePlatformUtils()
{
    CC_SAFE_DELETE(s_PlatformUtils);
    s_PlatformUtils = NULL;
}






