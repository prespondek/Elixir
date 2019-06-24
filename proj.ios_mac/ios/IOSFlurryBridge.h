//
//  IOSFlurry.h
//  Clear For Action
//
//  Created by Peter Respondek on 9/22/14.
//
//

#ifndef __Clear_For_Action__IOSFlurry__
#define __Clear_For_Action__IOSFlurry__

#include "FlurryBridge.h"

class IOSFlurryBridge : public FlurryBridge
{
    friend class FlurryBridge;
public:
    virtual ~IOSFlurryBridge();

    virtual void startSession (const char* flurryid);
    virtual void logEvent(const char* event, bool timed = false);
    virtual void logEvent(const char* event, std::map<std::string,std::string>& params, bool timed = false);
    virtual void endTimedEvent(const char* event, std::map<std::string,std::string>* params = NULL);
    
protected:
    IOSFlurryBridge();

};

#endif /* defined(__Clear_For_Action__IOSFlurry__) */
