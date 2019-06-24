package com.bridge.vungle;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import android.support.annotation.NonNull;
import android.util.Log;

import com.vungle.publisher.VungleAdEventListener;
import com.vungle.publisher.VungleInitListener;
import com.vungle.publisher.VunglePub;

public class VungleBridge {

    private static Cocos2dxActivity s_activity;

	private static final String TAG = "com.bridge.vungle";

    private static VungleAdEventListener _adlistener = new VungleAdEventListener() {
        @Override
        public void onAdEnd(@NonNull final String s, final boolean b, final boolean b1) {
            s_activity.runOnGLThread(new Runnable() {
                @Override
                public void run() { VungleBridge.onAdEnd(s,b,b1); }
            });
        }
        @Override
        public void onAdStart(@NonNull final String s) {
            s_activity.runOnGLThread(new Runnable() {
                @Override
                public void run() { VungleBridge.onAdStart(s); }
            });
        }
        @Override
        public void onUnableToPlayAd(@NonNull final String s, String s1) {
            s_activity.runOnGLThread(new Runnable() {
                @Override
                public void run() { VungleBridge.onAdUnavailable(s); }
            });
        }
        @Override
        public void onAdAvailabilityUpdate(@NonNull final String s, boolean b) {
            s_activity.runOnGLThread(new Runnable() {
                @Override
                public void run() { VungleBridge.onCachedAdAvailable(s); }
            });
        }
    };

    public static boolean isCachedAdAvailable( final String placementId ) {
        if (VunglePub.getInstance().isInitialized()) {
            return VunglePub.getInstance().isAdPlayable(placementId);
        }
        return false;
    }

	public static void init (final String appId, final String [] placementIds)
	{
		Log.v(TAG, "init() is called... appId = " + appId);
		s_activity = (Cocos2dxActivity) Cocos2dxActivity.getContext();
        final VunglePub vunglePub = VunglePub.getInstance();
        vunglePub.init(s_activity, appId, placementIds, new VungleInitListener() {
            @Override
            public void onSuccess() {
                VunglePub.getInstance().clearAndSetEventListeners(_adlistener);
                onVungleStart(true);
            }
            @Override
            public void onFailure(Throwable e){
                onVungleStart(false);

            }
        });
    }

	public static void playAd( final String placementId ) {
        Log.v(TAG, "play ad");
        s_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                VunglePub.getInstance().playAd(placementId, null);
            }
        });
	}

    public static void loadAd( final String placementId ) {
        Log.v(TAG, "load ad");
        VunglePub.getInstance().loadAd(placementId);
    }

    public static native void onVungleStart         ( boolean success );
    public static native void onAdStart             ( String placementID );
    public static native void onAdEnd               ( String placementID, boolean wasSuccessfulView, boolean wasCallToActionClicked );
    public static native void onCachedAdAvailable   ( String placementID );
    public static native void onAdUnavailable       ( String placementID );
}