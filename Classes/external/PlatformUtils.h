//
//  Downloader.hpp
//  Elixir
//
//  Created by Peter Respondek on 11/20/15.
//
//

#ifndef Downloader_hpp
#define Downloader_hpp

#include "cocos2d.h"


class PlatformUtils
{
public:

	static PlatformUtils* getInstance();
	void purgePlatformUtils();

    virtual void downloadUrl ( const std::string& url,
                               const std::string& path,
                               const std::function<void(const std::string& url,const std::string& path, bool success)>& func ) = 0;
    virtual void createUUID ( std::string& uuid ) = 0;
    virtual void getBuildNumber ( std::string& out_version, std::string& out_build ) = 0;

protected:
    virtual bool init() = 0;

    PlatformUtils();
	~PlatformUtils();

};

#endif /* Downloader_hpp */
