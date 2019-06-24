/****************************************************************************
 Copyright (c) 2012      Wenbin Wang
 
 http://geeksavetheworld.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

package com.bridge.chartboost;

import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.CBLocation;
import com.chartboost.sdk.ChartboostDelegate;

import com.chartboost.sdk.Libraries.CBLogging;
import com.chartboost.sdk.Model.CBError.CBImpressionError;

import java.lang.ref.WeakReference;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class ChartboostBridge {
    private static final String TAG = "com.bridge.chartboost";
    private static WeakReference<Cocos2dxActivity> s_activity;

    private static native boolean shouldDisplayInterstitial(String location);
    private static native boolean shouldRequestInterstitial(String location);
    private static native void didFailToLoadInterstitial(String location);
    private static native void didDismissInterstitial(String location);
    private static native void didCloseInterstitial(String location);
    private static native void didClickInterstitial(String location);
    private static native boolean shouldDisplayLoadingViewForMoreApps();
    private static native boolean shouldRequestMoreApps(String location);
    private static native boolean shouldDisplayMoreApps(String location);
    private static native void didFailToLoadMoreApps(String location);
    private static native void didDismissMoreApps(String location);
    private static native void didCloseMoreApps(String location);
    private static native void didClickMoreApps(String location);
    private static native void didCacheInterstitial(String location);
    private static native void didDisplayInterstitial(String location);
    
    // the method must be called in main thread, before any other method
    public static void install(){
    }
    
    public static void startWithAppId(final Cocos2dxActivity activity, final String appId, final String appSig) {
        Log.v(TAG, "setAppId() is called... appId = " + appId + " appSig = " + appSig);
        ChartboostBridge.s_activity = new WeakReference<Cocos2dxActivity>(activity);
        Chartboost.startWithAppId(activity,appId,"dd2d41b69ac01b80f443f5b6cf06096d457f82bd");
        Chartboost.onCreate(activity);
        Chartboost.setDelegate(ChartboostBridge.s_chartBoostDelegate);
    }

    public static void cacheInterstitial(final String location) {
        Log.v(TAG, "cacheInterstitial() is called...");
        ChartboostBridge.s_activity.get().runOnUiThread(new Runnable() {
            public void run() {
                Chartboost.cacheInterstitial(location);
            }
        });
    }
    
    public static void showInterstitial(final String location) {
        Log.v(TAG, "showInterstitial() is called...");
        ChartboostBridge.s_activity.get().runOnUiThread(new Runnable() {
            public void run() {
                Chartboost.showInterstitial(location);
            }
        });

    }
    
    public static boolean hasInterstitial(final String location) {
    	Log.v(TAG, "hasCachedInterstitial(\"" + location + "\") is called...");

    	return Chartboost.hasInterstitial(location);

    }
    
    public static void cacheMoreApps(final String location) {
        Log.v(TAG, "cacheMoreApps() is called...");
        
        ChartboostBridge.s_activity.get().runOnUiThread(new Runnable() {
            public void run() {
                Chartboost.cacheMoreApps(location);
            }
        });
        
    }
    
    public static void showMoreApps(final String location) {
        Log.v(TAG, "showMoreApps() is called...");
        ChartboostBridge.s_activity.get().runOnUiThread(new Runnable() {
            public void run() {
                Chartboost.showMoreApps(location);
            }
        });
    }
    
    static public ChartboostDelegate s_chartBoostDelegate = new ChartboostDelegate() {
        /**
         * Interstital
         */
        @Override
        public boolean shouldRequestInterstitial(final String location)
        {
            return ChartboostBridge.shouldRequestInterstitial(location);
        }

        @Override
        public void didInitialize() {
            Log.v(TAG, "Chartboost did Initialize...");
        }

        @Override
        public boolean shouldDisplayInterstitial(final String location)
        {
            return ChartboostBridge.shouldDisplayInterstitial(location);
        }
        
        @Override
        public void didCacheInterstitial(final String location)
        {
            ChartboostBridge.s_activity.get().runOnGLThread(new Runnable() {
                public void run() {
                    ChartboostBridge.didCacheInterstitial(location);
                }
            });
        }
        
        public void didDisplayInterstitial(final String location)
        {
            ChartboostBridge.s_activity.get().runOnGLThread(new Runnable() {
                public void run() {
                    ChartboostBridge.didDisplayInterstitial(location);
                }
            });
        }
        
        @Override
        public void didFailToLoadInterstitial(final String location, CBImpressionError error)
        {
            Log.v(TAG, "Chartboost did Fail To LoadInterstitial...");
            ChartboostBridge.s_activity.get().runOnGLThread(new Runnable() {
                public void run() {
                    ChartboostBridge.didFailToLoadInterstitial(location);
                }
            });
        }

        @Override
        public void didFailToLoadInPlay(String location, CBImpressionError error) {
            Log.v(TAG, "Chartboost did Fail To Load In Play...");
        }

        public void didFailToLoadRewardedVideo(String location, CBImpressionError error) {
            Log.v(TAG, "Chartboost did Fail To Load Rewarded Video...");
        }

        @Override
        public void didDismissInterstitial(final String location)
        {
            ChartboostBridge.s_activity.get().runOnGLThread(new Runnable() {
                public void run() {
                    ChartboostBridge.didDismissInterstitial(location);
                }
            });
        }
        
        @Override
        public void didCloseInterstitial(final String location)
        {
            ChartboostBridge.s_activity.get().runOnGLThread(new Runnable() {
                public void run() {
                    ChartboostBridge.didCloseInterstitial(location);
                }
            });
        }

        @Override
        public void didClickInterstitial(final String location)
        {
            ChartboostBridge.s_activity.get().runOnGLThread(new Runnable() {
                public void run() {
                    ChartboostBridge.didClickInterstitial(location);
                }
            });
        }

    };
}
