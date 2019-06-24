//
//  IOSFlurry.cpp
//  Clear For Action
//
//  Created by Peter Respondek on 9/22/14.
//
//

#include "IOSFlurryBridge.h"
#include "Flurry.h"

NSDictionary* makeDictionary (std::map<std::string,std::string>& params)
{
    NSString* objects [params.size()];
    NSString* keys [params.size()];
    int count = 0;
    for (auto i = params.begin(); i != params.end(); i++) {
        keys[count] = [NSString stringWithUTF8String: (*i).first.c_str()];
        objects[count] = [NSString stringWithUTF8String: (*i).second.c_str()];
        count++;
    }
    NSDictionary* dict = [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:objects count:count] forKeys:[NSArray arrayWithObjects:keys count:count]];
    return dict;
}

IOSFlurryBridge::~IOSFlurryBridge()
{
    
}

IOSFlurryBridge::IOSFlurryBridge()
{
    
}

void IOSFlurryBridge::startSession(const char *flurryid)
{
    [Flurry startSession:[NSString stringWithUTF8String:flurryid]];
}

void IOSFlurryBridge::logEvent(const char* event, bool timed)
{
    [Flurry logEvent:[NSString stringWithUTF8String:event] timed:timed];
}
void IOSFlurryBridge::logEvent(const char* event, std::map<std::string,std::string>& params, bool timed)
{
    NSDictionary* dict = makeDictionary(params);
    [Flurry logEvent:[NSString stringWithUTF8String:event] withParameters:dict];
    
}
void IOSFlurryBridge::endTimedEvent(const char* event, std::map<std::string,std::string>* params)
{
    NSDictionary* dict = nil;
    if (params) {
        dict = makeDictionary(*params);
    }
    [Flurry endTimedEvent:[NSString stringWithUTF8String:event] withParameters:dict];
}
