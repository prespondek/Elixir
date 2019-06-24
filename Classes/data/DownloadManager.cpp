//
//  Social.cpp
//  Elixir
//
//  Created by Peter Respondek on 9/15/15.
//
//

#include "../data/DownloadManager.h"
#include "../external/md5.h"
#include "../cocos_extensions/Lanyard_Util.h"
#include "../external/PlatformUtils.h"

DownloadManager::DownloadManager()
{}

DownloadManager::~DownloadManager()
{}

bool DownloadManager::init()
{
    FileUtils* file_util = FileUtils::getInstance();
    
    _cache_loc = file_util->getWritablePath() + "cache/";
    
    clearCache();
    
    return true;
}

void DownloadManager::clearCache()
{
    FileUtils* file_util = FileUtils::getInstance();

    file_util->removeDirectory(_cache_loc);
    file_util->createDirectory(_cache_loc);
    
    /*TextureCache* t_cache = Director::getInstance()->getTextureCache();
    
    for (auto i : _texture_cache) {
        i.second->release();
        t_cache->removeTexture(i.second);
    }
    _texture_cache.clear();*/
}

void DownloadManager::removeCallback(const std::string &url)
{
    _texture_queue.erase(url);
}

Texture2D* DownloadManager::downloadTexture(const std::string& url,
                                            const std::string& save_location,
                                            const std::function<void(Texture2D*)>& func)
{
    std::string path = _cache_loc + save_location;
    
    // first lets see if we haven't already downloaded a file at that location already
    /*auto loc = _texture_cache.find(path);
    if ( loc != _texture_cache.end()) {
        return loc->second;
    }*/
    
    // check to see if the texture is already in the texture cache;
    Texture2D* tex = Director::getInstance()->getTextureCache()->getTextureForKey(path);
    if (tex) return tex;
    
    // check if file is already on the disk.
    if (FileUtils::getInstance()->isFileExist(path)) {
        
        Director::getInstance()->getTextureCache()->addImageAsync(path, [this, path, func] (Texture2D* tex) {
            /*auto result = _texture_cache.insert(std::pair<std::string, Texture2D*>(path, tex));
            if (result.second) {
                tex->retain();
            }*/
            func(tex);
        });
        return nullptr;
    }
    
    auto result = _texture_queue.insert(std::pair<std::string,
                          std::vector<std::function<void(Texture2D*)>>>(url,
                                                                        std::vector<std::function<void(Texture2D*)>>()));
    
    // push our function onto the stack;
    result.first->second.push_back(func);
    
    // if our insert fails it means we have already started to download this file.
    if (!result.second) return nullptr;
    
    FileUtils* file_util = FileUtils::getInstance();
    
    // create our save directory if needed
    size_t pos = save_location.find_last_of("/");
    if (pos != std::string::npos) {
        file_util->createDirectory(_cache_loc + save_location.substr(0,pos));
    }
    
    PlatformUtils::getInstance()->downloadUrl(url, _cache_loc + save_location, [this] (std::string url, std::string path, bool success) {
        auto iter = _texture_queue.find(url);
        if (iter != _texture_queue.end()) {
            if (success) {
                TextureCache* t_cache = Director::getInstance()->getTextureCache();
                Texture2D* tex = t_cache->getTextureForKey(path);
                if (tex) t_cache->removeTexture(tex);
                t_cache->addImageAsync(path, [this, path, url] (Texture2D* tex) {
                    auto iter = _texture_queue.find(url);
                    if (iter != _texture_queue.end()) {
                        for (auto& i : iter->second) {
                            i(tex);
                        }
                        _texture_queue.erase(iter);
                    }
                    //auto result = _texture_cache.insert(std::pair<std::string, Texture2D*>(path, tex));
                    /*if (result.second) {
                        tex->retain();
                    }*/
                });
            } else {
                for (auto& i : iter->second) {
                    i(nullptr);
                }
                _texture_queue.erase(iter);
            }
        }
    });
    return nullptr;
}



void DownloadManager::makeGravatarUrl(const std::string &in_email, std::string &out_url) {

    std::string email = in_email;
    email.erase(email.begin(), std::find_if(email.begin(), email.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    email.erase(std::find_if(email.rbegin(), email.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), email.end());
    std::transform(email.begin(), email.end(), email.begin(), ::tolower);
    out_url = "http://www.gravatar.com/avatar/" + MD5(email).hexdigest() + ".jpg?s=128d=mm";
}

