//
//  GameAudioEngine.hpp
//  Elixir
//
//  Created by Peter Respondek on 2/9/16.
//
//

#ifndef GameAudioEngine_hpp
#define GameAudioEngine_hpp


#include "cocos2d.h"

USING_NS_CC;

#if !defined (AUDIO_DEBUG) || AUDIO_DEBUG == 0
#define AUDIOLOG(...)       do {} while (0)

#elif AUDIO_DEBUG == 1
#define AUDIOLOG(format, ...)      cocos2d::log(format, ##__VA_ARGS__)
#endif

#define USE_AUDIO_ENGINE 1
//#define USE_SIMPLE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

class GameAudioEngine
{
public:
    static GameAudioEngine* getInstance();
    //static CocosDenshion::SimpleAudioEngine* getAudioEngine();
    
    void setMusicVolume          ( float volume );
    void setSoundVolume            ( float volume );
    void queueSound              ( const std::string& sound_file);
    unsigned int playEffect      ( const char *pszFilePath, bool loop = false,
                            float pitch = 1.0f, float pan = 0.0f, float gain = 1.0f );
    void fadeMusic               ( float volume, float duration, std::function<void()> func = nullptr);
    void fadeSound               ( float volume, float duration, std::function<void()> func = nullptr);
    void pinSound                ( const std::string& key, uint32_t sound_id );
    uint32_t getPinnedSound      ( const std::string& key );
    uint32_t unpinSound          ( const std::string& key );
    void preloadEffect           ( const std::string& file );
    void unloadEffect            ( const std::string& file );
    void playBackgroundMusic     ( const char* filePath, bool loop = false );
    void stopEffect              ( unsigned int soundId );
    bool willPlayBackgroundMusic ( );
    bool isBackgroundMusicPlaying( );
    float getMusicVolume( );
    float getSoundVolume       ( );
    void pause                   ( );
    void resume                  ( );
    void end                     ( );
    
private:
    GameAudioEngine();
    
    std::set<std::string> _sound_queue;
    std::map<std::string,uint32_t> _sfx_pins;
    float _sound_volume;
    float _music_volume;

};

inline float GameAudioEngine::getSoundVolume() { return _sound_volume; }
inline float GameAudioEngine::getMusicVolume() { return _music_volume; }
 
#endif /* GameAudioEngine_hpp */
