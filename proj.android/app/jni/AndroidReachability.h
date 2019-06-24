#ifndef ANDROID_REACHABILITY_H
#define ANDROID_REACHABILITY_H

#include "external/Reachability.h"

class AndroidReachability : public Reachability
{
    bool checkReachabilityForInternetConnection ();


};

#endif
