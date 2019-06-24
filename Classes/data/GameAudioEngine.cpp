//
//  GameAudioEngine.cpp
//  Elixir
//
//  Created by Peter Respondek on 2/9/16.
//
//

#include "../data/GameAudioEngine.h"

static GameAudioEngine* g_aEngine;

GameAudioEngine::GameAudioEngine() :
_sound_volume(1.0f),
_music_volume(1.0f)
{
    
}

GameAudioEngine* GameAudioEngine::getInstance()
{
    if (! g_aEngine)
    {
        g_aEngine = new (std::nothrow) GameAudioEngine();
    }
    
    return g_aEngine;
}

void GameAudioEngine::preloadEffect(const std::string& file)
{
#if USE_AUDIO_ENGINE
    AudioEngine::preload(file);
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->preloadEffect(file.c_str());
#endif
}

void GameAudioEngine::unloadEffect(const std::string& file)
{
#if USE_AUDIO_ENGINE
    AudioEngine::uncache(file);
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->unloadEffect(file.c_str());
#endif
}

/*CocosDenshion::SimpleAudioEngine* GameAudioEngine::getAudioEngine()
{
    return CocosDenshion::SimpleAudioEngine::getInstance();
}*/

void GameAudioEngine::queueSound(const std::string &sound_file)
{
    _sound_queue.insert(sound_file);
    if (!Director::getInstance()->getScheduler()->isScheduled("queue sound", this)) {
        Director::getInstance()->getScheduler()->schedule([this] (float dt) {
            for ( auto sound : _sound_queue ) {
                GameAudioEngine::getInstance()->playEffect(sound.c_str());
#if USE_AUDIO_ENGINE
                AudioEngine::play2d(sound.c_str());
#elif USE_SIMPLE_AUDIO_ENGINE
                SimpleAudioEngine::getInstance()->playEffect(sound.c_str());
#endif
            }
            _sound_queue.clear();
        }, this, 0.0f, 0, 0.07f, false, "queue sound");
    }
}

void GameAudioEngine::stopEffect ( unsigned int soundId )
{
#if USE_AUDIO_ENGINE
    AudioEngine::stop     ( soundId );
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->stopEffect     ( soundId );
#endif
}

bool GameAudioEngine::isBackgroundMusicPlaying()
{
#if USE_AUDIO_ENGINE
    return AudioEngine::isMusicPlaying();
#elif USE_SIMPLE_AUDIO_ENGINE
    return SimpleAudioEngine::getInstance()->isBackgroundMusicPlaying     ( );
#endif
}

void GameAudioEngine::playBackgroundMusic     ( const char* filePath, bool loop )
{
#if USE_AUDIO_ENGINE
    AudioEngine::playMusic( filePath, loop, _music_volume );
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->playBackgroundMusic( filePath,loop );
#endif
}

unsigned int GameAudioEngine::playEffect(const char *pszFilePath, bool bLoop,
                                                float pitch, float pan, float gain)
{
    AUDIOLOG("GameAudioEngine::playEffect: %s", pszFilePath);
    gain = MIN(gain,_sound_volume);
#if USE_AUDIO_ENGINE
    return AudioEngine::play2d(pszFilePath, bLoop, gain);
#elif USE_SIMPLE_AUDIO_ENGINE
    return CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(pszFilePath, bLoop, pitch, pan, gain);
#endif
    
}

void GameAudioEngine::fadeMusic(float volume, float duration, std::function<void()> func)
{
    float delta_volume = getMusicVolume() - volume;
    float timer = duration;
    Director::getInstance()->getScheduler()->unschedule("music fade", this);
    Director::getInstance()->getScheduler()->schedule([this,volume,delta_volume,duration,timer,func] (float delta) mutable {
        timer -= delta;
        float new_volume = (timer / duration) * delta_volume + volume;
        setMusicVolume(new_volume);
        if (timer <= 0.0f) {
            setMusicVolume(volume);
            Director::getInstance()->getScheduler()->unschedule("music fade", this);
            if (func) { func(); }
        }
    }, this, 0.0f, CC_REPEAT_FOREVER, 0.0, 0.0f, "music fade");
}

void GameAudioEngine::fadeSound(float volume, float duration, std::function<void()> func)
{
    float delta_volume = getSoundVolume() - volume;
    float timer = duration;
    Director::getInstance()->getScheduler()->unschedule("sfx fade", this);
    Director::getInstance()->getScheduler()->schedule([this,volume,delta_volume,duration,timer,func] (float delta) mutable {
        timer -= delta;
        float new_volume = (timer / duration) * delta_volume + volume;
        setSoundVolume(new_volume);
        if (timer <= 0.0f) {
            setSoundVolume(volume);
            Director::getInstance()->getScheduler()->unschedule("sfx fade", this);
            if (func) { func(); }
        }
    }, this, 0.0f, CC_REPEAT_FOREVER, 0.0, 0.0f, "sfx fade");
}

void GameAudioEngine::setMusicVolume(float volume)
{
    _music_volume = clampf(volume, 0.0f, 1.0f);
#if USE_AUDIO_ENGINE
    AudioEngine::setMusicVolume(_music_volume);
#elif USE_SIMPLE_AUDIO_ENGINE
    CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(_music_volume);
#endif
}

void GameAudioEngine::setSoundVolume(float volume)
{
    _sound_volume = clampf(volume, 0.0f, 1.0f);
#if USE_AUDIO_ENGINE
    AudioEngine::setSoundVolume(_sound_volume);
#elif USE_SIMPLE_AUDIO_ENGINE
    CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(_sound_volume);
#endif
}

void GameAudioEngine::pinSound                ( const std::string& key, uint32_t sound_id )
{
    _sfx_pins.insert(std::pair<std::string, uint32_t>(key,sound_id));
}
uint32_t GameAudioEngine::getPinnedSound      ( const std::string& key )
{
    auto iter = _sfx_pins.find(key);
    if (iter != _sfx_pins.end()) {
        return iter->second;
    }
    return UINT32_MAX;

}
uint32_t GameAudioEngine::unpinSound              ( const std::string& key )
{
    auto iter = _sfx_pins.find(key);
    if (iter != _sfx_pins.end()) {
        uint32_t sound_id = iter->second;
        _sfx_pins.erase(iter);
        return sound_id;
    }
    return UINT32_MAX;
}

void GameAudioEngine::resume() {
#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}

void GameAudioEngine::pause() {
#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

void GameAudioEngine::end() {
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
}

bool GameAudioEngine::willPlayBackgroundMusic ( )
{
#if USE_AUDIO_ENGINE
    return true;
#elif USE_SIMPLE_AUDIO_ENGINE
    return SimpleAudioEngine::getInstance()->willPlayBackgroundMusic();
#endif
}


