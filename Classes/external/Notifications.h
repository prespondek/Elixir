//
//  LocalNotifications.h
//  Clear For Action
//
//  Created by Peter Respondek on 12/25/14.
//
//

#ifndef Clear_For_Action_LocalNotifications_h
#define Clear_For_Action_LocalNotifications_h

#include <time.h>

class Notifications
{
public:
    static void requestNotificationPremission ();
    static void postLocalNotification (time_t time, const std::string& str, const char* sound = NULL);
    static void cancelLocalNotifications ();
    static void setApplicationIconBadgeNumber (int num);
    static bool isNotificationPermitted ();
};

#endif
