package com.bridge.network;

import java.lang.ref.WeakReference;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

import static org.cocos2dx.lib.Cocos2dxHelper.*;

public class ReachabilityBridge extends BroadcastReceiver {

    private static ReachabilityBridge _instance;
    public ReachabilityBridge () { }
    private static WeakReference <Cocos2dxActivity>_context;

    public static void init (Cocos2dxActivity context)
    {
        if (_instance == null) {
            _instance = new ReachabilityBridge();
            _instance._context = new WeakReference<Cocos2dxActivity>(context);
        }
        context.registerReceiver(_instance,new IntentFilter("android.net.conn.CONNECTIVITY_CHANGE"));
    }

    public static void shutdown(Cocos2dxActivity context) {
        if (_instance != null) {
            context.unregisterReceiver(_instance);
        }
    }

    public static boolean isActivityConnected() {
        if (_instance != null) {
            return _instance.isConnected();
        }
        return false;
    }

    public static boolean isConnected() {
        ConnectivityManager manager = (ConnectivityManager) _context.get().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = manager.getActiveNetworkInfo();
        if (networkInfo != null && networkInfo.isConnected()) {
            return true;
        }
        return false;
    }

    public void onReceive(Context context, Intent intent) {
        Log.v("com.Lanyard.Pirateer", "ConnectivityChanged");
        if ( _instance == null )
            return;
        final boolean connected = isActivityConnected();
        _context.get().runOnGLThread(new Runnable() {
            public void run() {
                ReachabilityChanged(connected);
            }
        });
    }

    public static native void ReachabilityChanged(boolean connected);
}

    