//
//  IOSGameCenter.h
//  Pirateer
//
//  Created by Peter Respondek on 3/10/14.
//
//

#ifndef Pirateer_IOSGameCenter_h
#define Pirateer_IOSGameCenter_h

#include "GameCenter.h"
#include "cocos2d.h"

class IOSGameCenter : public GameCenter
{
public:
    bool init();

    void login();
    bool isLoggedIn();
    
    bool showAchievements();
    void postAchievement(const char* idName, int percentComplete);
    void clearAllAchievements();
    
    CCString* getPlayerId ();
    
    bool showScores();
    void postScore(const char* idName, int score);
    void clearAllScores();
    
    void retrieveScoresFromDevice();
protected:
    virtual ~IOSGameCenter();
    
};
#endif
