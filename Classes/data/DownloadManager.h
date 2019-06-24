//
//  Social.h
//  Elixir
//
//  Created by Peter Respondek on 9/15/15.
//
//

#ifndef __Elixir__Social__
#define __Elixir__Social__

#include "cocos2d.h"

USING_NS_CC;

class DownloadManager : public Ref
{
public:
    CREATE_FUNC(DownloadManager);
    virtual ~DownloadManager();
    void clearCache ();
    std::string getCacheLocation () { return _cache_loc; }
    void removeCallback( const std::string& url );
    Texture2D* downloadTexture(const std::string& url, const std::string& save_location, const std::function<void(Texture2D*)>& func );
    
protected:
    DownloadManager();
    bool init ();
    void makeGravatarUrl(const std::string& in_email, std::string& out_url);
    
    std::string _cache_loc;

    //std::map<std::string,Texture2D*> _texture_cache;
    std::map<std::string,std::vector<std::function<void(Texture2D*)>>> _texture_queue;
    
    
};

#endif /* defined(__Elixir__Social__) */
