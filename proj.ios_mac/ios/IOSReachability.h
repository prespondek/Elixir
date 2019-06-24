
#ifndef __Pirateer__CCIOSReachability__
#define __Pirateer__CCIOSReachability__

#include "cocos2d.h"
#include "../Classes/external/Reachability.h"

USING_NS_CC;

class IOSReachability : public Reachability
{
public:
    bool init();
    
    bool checkReachabilityForInternetConnection ();
    void monitorReachabilityWithHostName ( const char* host_name );
protected:
    virtual ~IOSReachability();
};

#endif /* defined(__Pirateer__CCIOSReachability__) */


