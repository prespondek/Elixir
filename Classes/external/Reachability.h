//
//  Reachability.h
//  Pirateer
//
//  Created by Peter Respondek on 6/24/14.
//
//

#ifndef __Pirateer__Reachability__
#define __Pirateer__Reachability__

#define NOTIFICATION_NET_OFFLINE "Network Down"
#define NOTIFICATION_NET_ONLINE "Network Up"

#include <vector>
#include "cocos2d.h"


enum ReachabilityStatus {
	NetNotReachable = 0,
	NetReachableViaWiFi,
	NetReachableViaWWAN
};

class ReachabilityDelegate
{
public:
    ReachabilityDelegate();
    virtual ~ReachabilityDelegate();
    virtual void reachabilityStatusChanged( ReachabilityStatus status ) {};
};

class Reachability
{
public:
    
    static Reachability* getInstance                    ( );
    void purgeReachability                              ( );

    virtual bool init                                   ( );
    //virtual void checkReachabilityWithHostName ( const char* host_name );
    virtual bool checkReachabilityForInternetConnection ( );
    //virtual bool checkreachabilityForLocalWiFi ();
    
    virtual void monitorReachabilityWithHostName        ( const char* host_name );
    void reachabilityStatusChanged                      ( ReachabilityStatus status );
    
    bool addDelegate                                    ( ReachabilityDelegate* delegate );
    bool removeDelegate                                 ( ReachabilityDelegate* delegate );
    const std::vector<ReachabilityDelegate*>& getDelegates ( );

protected:
    virtual ~Reachability                               ( );
    //Array* _delegates = NULL;
    std::vector<ReachabilityDelegate*> _delegates;
    //CCReachabilityDelegate* m_delegate = NULL;
};

inline const std::vector<ReachabilityDelegate*>& Reachability::getDelegates() { return _delegates; }

#endif /* defined(__Pirateer__Reachability__) */
