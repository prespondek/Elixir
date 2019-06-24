//
//  CCIOSReachability.h
//  Pirateer
//
//  Created by Peter Respondek on 6/24/14.
//
//

#ifndef __Pirateer__CCIOSReachability__
#define __Pirateer__CCIOSReachability__

#include "cocos2d.h"
#include "CCReachability.h"

USING_NS_CC;

class CCIOSReachability : public CCReachability
{
public:
    bool init();
    
    bool checkReachabilityForInternetConnection ();
    void monitorReachabilityWithHostName ( const char* host_name );
protected:
    virtual ~CCIOSReachability();
};

#endif /* defined(__Pirateer__CCIOSReachability__) */
