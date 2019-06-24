//
//  Reachability.cpp
//  Pirateer
//
//  Created by Peter Respondek on 6/24/14.
//
//

#include "../external/Reachability.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "../../proj.ios_mac/ios/IOSReachability.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../../proj.android/app/jni/AndroidReachability.h"
#endif

Reachability* g_Reachability = nullptr;

Reachability::~Reachability()
{
}

void Reachability::purgeReachability()
{
    delete g_Reachability;
    g_Reachability = nullptr;
}

Reachability* Reachability::getInstance()
{
    if (!g_Reachability) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        g_Reachability = new IOSReachability();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        g_Reachability = new AndroidReachability();
#else
        g_Reachability = new CCReachability();
#endif
    }
    if (g_Reachability && g_Reachability->init()) {
        g_Reachability->monitorReachabilityWithHostName("www.apple.com");
        return g_Reachability;
    }
    return nullptr;
}

void Reachability::reachabilityStatusChanged( ReachabilityStatus status ) {
    for (auto i : _delegates) {
        i->reachabilityStatusChanged(status);
    }
}

bool Reachability::checkReachabilityForInternetConnection()
{
	return false;
}

void Reachability::monitorReachabilityWithHostName ( const char* host_name )
{

}


bool Reachability::addDelegate(ReachabilityDelegate *delegate)
{
    auto iter = std::find(_delegates.begin(), _delegates.end(), delegate);
    if (iter != _delegates.end()) return false;
    _delegates.push_back(delegate);
    return true;
}

bool Reachability::removeDelegate(ReachabilityDelegate *delegate)   {
    auto iter = std::find(_delegates.begin(), _delegates.end(), delegate);
    if (iter == _delegates.end()) return false;
    _delegates.erase(iter);
    return true;
}

ReachabilityDelegate::ReachabilityDelegate() {
    Reachability::getInstance()->addDelegate(this);
}

ReachabilityDelegate::~ReachabilityDelegate() {
    Reachability::getInstance()->removeDelegate(this);
}

bool Reachability::init()
{
    return true;
}


