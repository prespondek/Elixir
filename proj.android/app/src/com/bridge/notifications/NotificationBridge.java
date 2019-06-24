package com.bridge.notifications;

import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;

import org.cocos2dx.cpp.AppActivity;
import org.cocos2dx.lib.Cocos2dxActivity;

import com.Lanyard.Elixir.R;

import android.app.AlarmManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.util.Log;

public class NotificationBridge extends BroadcastReceiver 
{
	
	private static final String TAG = "com.bridge.notifications";
    public static final String PREFS_NAME = "NotificationsPrefs";
    public static int notification_count = 0;
	
	public void onReceive(Context context, Intent paramIntent) 
	{
		Log.v(TAG, "Scheduled Alarm Recived at: " + System.currentTimeMillis());
		
		String notification = paramIntent.getStringExtra ("notification");
		NotificationCompat.Builder mBuilder =
    	        new NotificationCompat.Builder(context)
    	        .setSmallIcon(R.mipmap.icon)
    	        .setContentTitle("Clear For Action")
    	        .setContentText(notification);
    	// Creates an explicit intent for an Activity in your app
    	Intent resultIntent = new Intent(context, AppActivity.class);
    	

    	// The stack builder object will contain an artificial back stack for the
    	// started Activity.
    	// This ensures that navigating backward from the Activity leads out of
    	// your application to the Home screen.
    	TaskStackBuilder stackBuilder = TaskStackBuilder.create(context);
    	// Adds the back stack for the Intent (but not the Intent itself)
    	stackBuilder.addParentStack(AppActivity.class);
    	// Adds the Intent that starts the Activity to the top of the stack
    	stackBuilder.addNextIntent(resultIntent);
    	PendingIntent resultPendingIntent =
    	        stackBuilder.getPendingIntent(
    	            0,
    	            PendingIntent.FLAG_UPDATE_CURRENT
    	        );
    	mBuilder.setContentIntent(resultPendingIntent);
    	NotificationManager mNotificationManager =
    	    (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
    	// mId allows you to update the notification later on.
    	mNotificationManager.notify(1, mBuilder.build());
    	
	}
	
	public static void scheduleNotification (final String notification, long time)
	{		
		SharedPreferences settings = (Cocos2dxActivity.getContext()).getSharedPreferences(PREFS_NAME, 0);
	    SharedPreferences.Editor editor = settings.edit();
	    
	    notification_count++;
	    editor.putInt("NotificationCount", notification_count);
	    editor.commit();
		
		Calendar calendar = Calendar.getInstance();
		calendar.setTimeInMillis(time * 1000);
	
		Cocos2dxActivity.getContext();
		AlarmManager alarmManager = (AlarmManager) Cocos2dxActivity.getContext().getSystemService(Context.ALARM_SERVICE);
		Intent intent = new Intent(Cocos2dxActivity.getContext(), NotificationBridge.class);
		intent.putExtra ("notification", notification);
		PendingIntent pendingIntent = PendingIntent.getBroadcast(Cocos2dxActivity.getContext(), notification_count, intent, PendingIntent.FLAG_UPDATE_CURRENT);
		alarmManager.set(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), pendingIntent);
	}
	
	public static void clearNotifications ()
	{
		SharedPreferences settings = (Cocos2dxActivity.getContext()).getSharedPreferences(PREFS_NAME, 0);
		notification_count = settings.getInt("NotificationCount", 0);
		if (notification_count == 0) return;
		
		AlarmManager alarmManager = (AlarmManager) Cocos2dxActivity.getContext().getSystemService(Context.ALARM_SERVICE);
		Intent intent = new Intent(Cocos2dxActivity.getContext(), NotificationBridge.class);
		for (int i = 1; i <= notification_count; i++) {
			PendingIntent pendingIntent = PendingIntent.getBroadcast(Cocos2dxActivity.getContext(), i, intent, 0);
			alarmManager.cancel(pendingIntent);
			pendingIntent.cancel();
	    }
		//pendingNotifications.clear();
	}
}