//
//  Downloader.hpp
//  Elixir
//
//  Created by Peter Respondek on 11/20/15.
//
//

#ifndef Android_PlatformUtils_h
#define Android_PlatformUtils_h

#include "external/PlatformUtils.h"


class AndroidPlatformUtils : public PlatformUtils
{
	friend class PlatformUtils;
public:
    void downloadUrl 		( const std::string& url, const std::string& path, const std::function<void(const std::string& url,const std::string& path, bool success)>& func );
    void createUUID 		( std::string& uuid );
    void getBuildNumber 	( std::string& out_version, std::string& out_build );
    void downloadCallback	( const std::string& url, const std::string& path, bool success );

protected:
    bool init						( );
    AndroidPlatformUtils			( );
	virtual ~AndroidPlatformUtils	( );

	std::map<std::string, std::function<void(const std::string& url,const std::string& path, bool success)>> _func_list;

};

#endif /* Downloader_hpp */
