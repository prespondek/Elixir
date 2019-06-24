//
//  IOSPlatformUtils.h
//  Elixir
//
//  Created by Peter Respondek on 3/17/16.
//
//

#ifndef IOSPlatformUtils_h
#define IOSPlatformUtils_h

#include "../Classes/external/PlatformUtils.h"


class IOSPlatformUtils : public PlatformUtils
{
    friend class PlatformUtils;
public:
    void downloadUrl ( const std::string& url, const std::string& path, const std::function<void(const std::string& url,const std::string& path, bool success)>& func );
    void createUUID ( std::string& uuid );
    void getBuildNumber ( std::string& out_version, std::string& out_build );

    virtual bool init();

    
};

#endif /* IOSPlatformUtils_h */
