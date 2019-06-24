//
//  CCLocalNotification.cpp
//  Pirateer
//
//  Created by Peter Respondek on 12/16/13.
//
//

#import <UIKit/UIKit.h>
#include "../Classes/external/Notifications.h"

void Notifications::requestNotificationPremission ()
{
    
    UIUserNotificationType userNotificationTypes = (UIUserNotificationTypeAlert |
                                                    UIUserNotificationTypeBadge |
                                                    UIUserNotificationTypeSound);
    UIUserNotificationSettings *settings = [UIUserNotificationSettings settingsForTypes:userNotificationTypes
                                                                             categories:nil];
    [[UIApplication sharedApplication] registerUserNotificationSettings:settings];
    [[UIApplication sharedApplication] registerForRemoteNotifications];
}

bool Notifications::isNotificationPermitted ()
{
    return [[UIApplication sharedApplication] isRegisteredForRemoteNotifications];
}

void Notifications::postLocalNotification (time_t time, const std::string& str, const char* sound)
{
    NSDate *itemDate = [NSDate dateWithTimeIntervalSince1970:(time)];
    
    UILocalNotification *localNotif = [[UILocalNotification alloc] init];
    if (localNotif == nil)
        return;
    localNotif.fireDate = itemDate;
    localNotif.timeZone = [NSTimeZone defaultTimeZone];
    NSString *alert = [NSString stringWithCString:(str) encoding:(NSASCIIStringEncoding)];
    localNotif.alertBody=alert;
    // Set the action button
    localNotif.alertAction = @"View";
    NSLog(@"setAlarm Called : %@",itemDate);
    //localNotif.hasAction = NO;
    if (sound) {
        localNotif.soundName = [NSString stringWithCString:(sound) encoding:(NSASCIIStringEncoding)];
    } else {
        localNotif.soundName = UILocalNotificationDefaultSoundName;
    }
    localNotif.applicationIconBadgeNumber = [UIApplication sharedApplication].applicationIconBadgeNumber+1;
    localNotif.repeatInterval = NSWeekCalendarUnit;
    //[UIApplication sharedApplication].applicationIconBadgeNumber += 1;
    // Specify custom data for the notification
    NSDictionary *infoDict = [NSDictionary dictionaryWithObject:@"someValue" forKey:@"someKey"];
    localNotif.userInfo = infoDict;
    
    // Schedule the notification
    [[UIApplication sharedApplication] scheduleLocalNotification:localNotif];
    [localNotif release];
}

void Notifications::cancelLocalNotifications ()
{
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
}

void Notifications::setApplicationIconBadgeNumber (int num)
{
    [[UIApplication sharedApplication] setApplicationIconBadgeNumber: num];
}
