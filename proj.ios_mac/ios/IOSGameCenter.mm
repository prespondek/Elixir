#include "IOSGameCenter.h"

#import <GameKit/GameKit.h>
#import "AppController.h"
#import "RootViewController.h"
#import <Crashlytics/Crashlytics.h>

@interface IOSGameCenterBridge : NSObject
{
}

+ (IOSGameCenterBridge*)shared;

- (void)login:(IOSGameCenter*)gameCenter;
- (BOOL)isLoggedIn;

- (BOOL)showAchievements;
- (void)postAchievement:(const char*)idName percent:(NSNumber*)percentComplete;
- (void)clearAllAchivements;

- (BOOL)showScores;
- (NSString*)getPlayerId;
- (void)postScore:(const char*)idName score:(NSNumber*)score;
- (void)clearAllScores;
@end

@interface IOSGameCenterBridge(intern)
- (void)saveAchievementToDevice:(GKAchievement*)achievement;
- (void)retrieveAchievementsFromDevice;

- (void)saveScoreToDevice:(GKScore*)score;
- (void)retrieveScoresFromDevice;

- (void)registerForAuthenticationNotification;
- (void)authenticationChanged;

- (NSString*)getGameCenterSavePath:(NSString*)postfix;
@end

@implementation IOSGameCenterBridge

static NSString* scoresArchiveKey = @"scores";
static NSString* scoresArchiveKeyLow = @"low";
static NSString* scoresArchiveKeyHigh = @"high";
static NSString* achievementsArchiveKey = @"achievements";

static IOSGameCenterBridge* instance = nil;

+ (IOSGameCenterBridge*)shared
{
    @synchronized(self) {
        if (instance == nil) {
            instance = [[self alloc] init];
            [instance registerForAuthenticationNotification];
        }
    }
    return instance;
}

- (void)login:(IOSGameCenter*)gameCenter
{
    
    GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];
    if (localPlayer.isAuthenticated) {
        return;
    }
    if ([GKLocalPlayer instancesRespondToSelector:@selector(setAuthenticateHandler:)]) {
        [localPlayer setAuthenticateHandler:^(UIViewController *viewController, NSError *error) {
            if (error) {
                if (error.code != GKErrorAuthenticationInProgress) {
                    NSLog(@"[GameCenter] login failed: %@", error.localizedDescription);
                }
                gameCenter->loginComplete(false);
                return;
            }
            if (viewController) {
                AppController* appController = (AppController*) [UIApplication sharedApplication].delegate;
                RootViewController* rootview = appController.viewController;
                [rootview presentViewController:viewController
                                       animated:YES
                                     completion:^{
                    
                }];
            } else {
                if (localPlayer.isAuthenticated) {
                    [self retrieveScoresFromDevice];
                    [self retrieveAchievementsFromDevice];
                    gameCenter->loginComplete(true);
                } else {
                    gameCenter->loginComplete(false);
                }
            }
        }];
    } else {
        [localPlayer authenticateWithCompletionHandler:^(NSError* error) {
            if (error) {
                if (error.code != GKErrorAuthenticationInProgress) {
                    NSLog(@"[GameCenter] login failed: %@", error.localizedDescription);
                }
                gameCenter->loginComplete(false);
            } else {
                [self retrieveScoresFromDevice];
                [self retrieveAchievementsFromDevice];
                gameCenter->loginComplete(true);
            }
        }];
    }
}
-(NSString*)getPlayerId
{
    GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];
    return localPlayer.playerID;
}

-(BOOL)isLoggedIn
{
    GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];
    return localPlayer.isAuthenticated;
}

#pragma mark -
#pragma mark Achievements

- (BOOL)showAchievements
{
    if (![GKLocalPlayer localPlayer].isAuthenticated) {
        return NO;
    }
    
    AppController* appController = (AppController*) [UIApplication sharedApplication].delegate;
    
    GKAchievementViewController* gkController = [[[GKAchievementViewController alloc] init] autorelease];
    gkController.achievementDelegate = appController.viewController;
    
    [appController.viewController presentModalViewController:gkController animated:YES];
    return YES;
}

- (void)postAchievement:(const char*)idName percent:(NSNumber*)percentComplete
{
    GKAchievement* achievement = [[GKAchievement alloc] initWithIdentifier:[NSString stringWithUTF8String:idName]];
    achievement.percentComplete = [percentComplete doubleValue];
    achievement.showsCompletionBanner = YES;
    
    NSArray* achievements = [NSArray arrayWithObject:achievement];
    
    if (![GKLocalPlayer localPlayer].isAuthenticated) {
        [self saveAchievementToDevice:achievement];
        return;
    }
    
    [GKAchievement reportAchievements:achievements withCompletionHandler:^(NSError* error) {
        if (error) {
            NSLog(@"[GameCenter] postAchievement for %s failed: %@", idName, error.localizedDescription);
        }
    }];
}

- (void)clearAllAchivements
{
    // clear cached achievements
    NSString* savePath = [self getGameCenterSavePath:achievementsArchiveKey];
    NSError* error;
    [[NSFileManager defaultManager] removeItemAtPath:savePath error:&error];
    
    // cleare remote achievements
    [GKAchievement resetAchievementsWithCompletionHandler:^(NSError* error) {
        if (error) {
            NSLog(@"[GameCenter] clearAllAchivements failed: %@", error.localizedDescription);
        }
    }];
}

#pragma mark -
#pragma mark Leaderboard

- (BOOL)showScores
{
    if (![GKLocalPlayer localPlayer].isAuthenticated) {
        return NO;
    }
    
    AppController* appController = (AppController*) [UIApplication sharedApplication].delegate;
    
    GKLeaderboardViewController* gkController = [[[GKLeaderboardViewController alloc] init] autorelease];
    gkController.timeScope = GKLeaderboardTimeScopeAllTime;
    gkController.leaderboardDelegate = appController.viewController;
    
    [appController.viewController presentModalViewController:gkController animated:YES];
    return YES;
}

- (void)postScore:(const char*)idName score:(NSNumber*)score;
{
    GKScore* gkScore = nil;
    @try {
        gkScore = [[[GKScore alloc] initWithLeaderboardIdentifier:[NSString stringWithUTF8String:idName]] autorelease];
    } @catch (NSException* e) {
        CLS_LOG(@"postScore: initLeaderboard Exception: %@ idName: %s score: %@", e, idName, score);
        return;
    }
    
    gkScore.value = [score longLongValue];
    gkScore.shouldSetDefaultLeaderboard = YES;
    
    NSArray* scores = [NSArray arrayWithObject:gkScore];

    
    if (![GKLocalPlayer localPlayer].isAuthenticated) {
        [self saveScoreToDevice:gkScore];
        return;
    }
        
    [GKScore reportScores:scores withCompletionHandler:^(NSError* error) {
        if (error) {
            NSLog(@"[GameCenter] postScore for %s failed: %@", idName, error.localizedDescription);
        }
    }];
    
}

- (void)clearAllScores
{
    // clear cached scores
    NSString* savePath = [self getGameCenterSavePath:scoresArchiveKey];
    NSError* error = nil;
    [[NSFileManager defaultManager] removeItemAtPath:savePath error:&error];
    
    // clear remote scores
    NSLog(@"[GameCenter] WARNING! clearAllScores is not supported on this platform");
}

#pragma mark -
#pragma mark Cache Achievements

- (void)saveAchievementToDevice:(GKAchievement*)achievement
{
    NSString* key = achievement.identifier;
    NSNumber* newValue = [NSNumber numberWithDouble:achievement.percentComplete];
    NSString* savePath = [self getGameCenterSavePath:achievementsArchiveKey];
    
    NSMutableDictionary* data = nil;
    if ([[NSFileManager defaultManager] fileExistsAtPath:savePath]) {
        data = [[[NSMutableDictionary alloc] initWithContentsOfFile:savePath] autorelease];
    } else {
        data = [[[NSMutableDictionary alloc] init] autorelease];
    }
    
    NSNumber* oldValue = [data objectForKey:key];
    if (oldValue == nil || [oldValue doubleValue] < [newValue doubleValue]) {
        [data setObject:newValue forKey:key];
        [data writeToFile:savePath atomically:YES];
    }
}

- (void)retrieveAchievementsFromDevice
{
    NSString* savePath = [self getGameCenterSavePath:achievementsArchiveKey];
    if (![[NSFileManager defaultManager] fileExistsAtPath:savePath]) {
        return;
    }
    
    NSMutableDictionary* data = [NSMutableDictionary dictionaryWithContentsOfFile:savePath];
    if (!data) {
        return;
    }
    
    NSError* error = nil;
    [[NSFileManager defaultManager] removeItemAtPath:savePath error:&error];
    
    NSMutableArray* achievements = [NSMutableArray array];
    
    for (NSString* key in data) {
        NSNumber* number = [data objectForKey:key];
        
        GKAchievement* achievement = [[[GKAchievement alloc] initWithIdentifier:key] autorelease];
        achievement.percentComplete = [number doubleValue];
        achievement.showsCompletionBanner = YES;
        
        [achievements addObject:achievement];
    }
    [GKAchievement reportAchievements:achievements withCompletionHandler:^(NSError* error){
        if (error) {
            for (GKAchievement* key in achievements) {
                [self saveAchievementToDevice:key];
            }
        }
    }];
}

#pragma mark -
#pragma mark Cache Leaderboard

- (void)saveScoreToDevice:(GKScore*)score
{
    NSString* key = score.category;
    NSNumber* newValue = [NSNumber numberWithLongLong:score.value];
    NSString* savePath = [self getGameCenterSavePath:scoresArchiveKey];
    
    NSMutableDictionary* data = nil;
    if ([[NSFileManager defaultManager] fileExistsAtPath:savePath]) {
        data = [[[NSMutableDictionary alloc] initWithContentsOfFile:savePath] autorelease];
    } else {
        data = [[[NSMutableDictionary alloc] init] autorelease];
    }
    
    NSNumber* lowValue = nil;
    NSMutableDictionary* lowData = nil;
    if ([data objectForKey:scoresArchiveKeyLow]) {
        lowData = [data objectForKey:scoresArchiveKeyLow];
        lowValue = [lowData objectForKey:key];
    } else {
        lowData = [[[NSMutableDictionary alloc] init] autorelease];
    }
    
    NSNumber* highValue = nil;
    NSMutableDictionary* highData = nil;
    if ([data objectForKey:scoresArchiveKeyHigh]) {
        highData = [data objectForKey:scoresArchiveKeyHigh];
        highValue = [highData objectForKey:key];
    } else {
        highData = [[[NSMutableDictionary alloc] init] autorelease];
    }
    
    // we don't have anything cached yet => cache as new high
    // or there is something cached and the new score is a new high
    if ((lowValue == nil && highValue == nil)
        ||  (highValue == nil || [highValue longLongValue] < [newValue longLongValue])) {
        [highData setObject:newValue forKey:key];
        [data setObject:highData forKey:scoresArchiveKeyHigh];
        [data writeToFile:savePath atomically:YES];
        return;
    }
    
    // there is something cached and the new score is a new low
    if (lowValue == nil || [lowValue longLongValue] > [newValue longLongValue]) {
        [lowData setObject:newValue forKey:key];
        [data setObject:lowData forKey:scoresArchiveKeyLow];
        [data writeToFile:savePath atomically:YES];
        return;
    }
}

- (void)retrieveScoresFromDevice
{
    NSString* savePath = [self getGameCenterSavePath:scoresArchiveKey];
    if (![[NSFileManager defaultManager] fileExistsAtPath:savePath]) {
        return;
    }
    
    NSMutableDictionary* data = [NSMutableDictionary dictionaryWithContentsOfFile:savePath];
    if (!data) {
        return;
    }
    
    NSError* error = nil;
    [[NSFileManager defaultManager] removeItemAtPath:savePath error:&error];
    
    NSMutableArray* scores = [NSMutableArray array];
    
    for (NSString* lowOrHighKey in [NSArray arrayWithObjects:scoresArchiveKeyLow,scoresArchiveKeyHigh,nil]) {
        NSMutableDictionary* lowOrHighData = [data objectForKey:lowOrHighKey];
        
        for (NSString* key in lowOrHighData) {
            NSNumber* number = [lowOrHighData objectForKey:key];
            
            GKScore* gkScore = [[[GKScore alloc] initWithLeaderboardIdentifier:key] autorelease];
            gkScore.value = [number longLongValue];
            gkScore.shouldSetDefaultLeaderboard = YES;
            
            [scores addObject:gkScore];
        }
    }
    [GKScore reportScores:scores withCompletionHandler:^(NSError* error) {
        if (error) {
            for (GKScore* key in scores) {
                [self saveScoreToDevice:key];
            }
        }
    }];
}

#pragma mark -
#pragma mark Cache Helper

- (NSString*)getGameCenterSavePath:(NSString*)postfix
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return [NSString stringWithFormat:@"%@/cfa.gamecenter.%@.cache",[paths objectAtIndex:0],postfix];
}

- (void)registerForAuthenticationNotification
{
    NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
    [nc addObserver: self selector:@selector(authenticationChanged) name:GKPlayerAuthenticationDidChangeNotificationName object:nil];
}

- (void)authenticationChanged
{
    if ([GKLocalPlayer localPlayer].isAuthenticated) {
        [self retrieveScoresFromDevice];
        [self retrieveAchievementsFromDevice];
    }
}

@end

bool IOSGameCenter::init() {
    return true;
}

void IOSGameCenter::login()
{
    if (m_gc_state == LOGGING_IN) return;
    if ([[IOSGameCenterBridge shared] isLoggedIn]) {
    } else {
        m_gc_state = LOGGING_IN;
        [[IOSGameCenterBridge shared] login:this];
    }
}

bool IOSGameCenter::isLoggedIn()
{
    return [[IOSGameCenterBridge shared] isLoggedIn];
}

bool IOSGameCenter::showAchievements()
{
    return [[IOSGameCenterBridge shared] showAchievements];
}

void IOSGameCenter::postAchievement(const char* idName, int percentComplete)
{
    if (!m_enabled) return;
    [[IOSGameCenterBridge shared] postAchievement:idName percent:[NSNumber numberWithInt:percentComplete]];
}

void IOSGameCenter::clearAllAchievements()
{
    [[IOSGameCenterBridge shared] clearAllAchivements];
}

bool IOSGameCenter::showScores()
{
    return [[IOSGameCenterBridge shared] showScores];
}

void IOSGameCenter::postScore(const char* idName, int score)
{
    if (!m_enabled) return;
    [[IOSGameCenterBridge shared] postScore:idName score:[NSNumber numberWithInt:score]];
}

void IOSGameCenter::clearAllScores()
{
    [[IOSGameCenterBridge shared] clearAllScores];
}

CCString* IOSGameCenter::getPlayerId()
{
    NSString* playerId = [[IOSGameCenterBridge shared] getPlayerId];
    if (!playerId) return NULL;
    return CCString::create( [playerId UTF8String] );
}

IOSGameCenter::~IOSGameCenter()
{
    [[IOSGameCenterBridge shared] dealloc];
}




