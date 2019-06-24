package com.bridge.appstore;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClient.BillingResponse;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.example.billingmodule.billing.BillingManager;
import com.example.billingmodule.billing.BillingManager.BillingUpdatesListener;
import com.example.billingmodule.billing.BillingManager.ServiceConnectedListener;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;


public class AndroidAppStore implements BillingUpdatesListener, SkuDetailsResponseListener, ServiceConnectedListener
{
    private static AndroidAppStore instance;
    static final String TAG = "com.bridge.appstore";
    public static BillingManager mBillingManager = null;
    private Map<String, Boolean> pendingItemData = new HashMap<String, Boolean>();


    public static AndroidAppStore getInstance() {
        if ( instance == null ) throw new RuntimeException("Call startAppStore before getting an instance");
        return instance;
    }

    public static void startAppStore( Activity activity, final String key ) {
        instance = new AndroidAppStore( activity, key );
    }

    private AndroidAppStore(Activity activity , final String key) {
        mBillingManager = new BillingManager(activity, this);
        mBillingManager.setKey(key);
    }

    private static String clearTitle(String title)
    {
        // "Iap Title (APP NAME)" ==> "Iap Title"
        int substrPos = title.lastIndexOf("(") - 1;
        if (substrPos > 0) {
            title = title.substring(0, substrPos);
        }
        return title;
    }

    public void onSkuDetailsResponse(@BillingResponse int responseCode, final List<SkuDetails> skuDetailsList)
    {
        if (responseCode != 0) {
            Log.e(TAG, "onQueryInventoryFinished failed: " + responseCode);
            AndroidAppStore.onProductRequestFailed();
            return;
        }
        if (skuDetailsList.isEmpty()) {
            Log.e(TAG, "onQueryInventoryFinished failed: Not a single detail returned! Google Play configured?");
            return;
        }

        Cocos2dxHelper.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                for (SkuDetails sku : skuDetailsList) {
                    onProductData(sku.getSku(),
                            clearTitle(sku.getTitle()),
                            sku.getDescription(),
                            sku.getPrice(),
                            0.0f);
                }
                AndroidAppStore.onProductRequestSuccess();
            }
        });
    }

    public void onBillingClientSetupFinished()
    {
        //startProductDataRequest();
        Cocos2dxHelper.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                onServiceStarted();
            }
        });
    }

    public void onConsumeFinished(String token, @BillingClient.BillingResponse int result)
    {
        delegateOnTransactionEnd();
    }

    public void onPurchasesUpdated(final List<Purchase> purchases)
    {
        Cocos2dxHelper.runOnGLThread(new Runnable() {
        @Override
        public void run() {
            for (final Purchase i : purchases) {
                if (onPurchaseSucceed(i.getSku())) {
                    mBillingManager.getActivity().runOnUiThread(new Runnable() {
                        public void run() {
                            mBillingManager.consumeAsync(i.getPurchaseToken());
                        }
                    });
                }
            }
        }});
    }

    public void onPurchasesFailed(@BillingResponse int result)
    {
        onPurchaseFail(result);
    }


    public void onServiceConnected(@BillingResponse int resultCode)
    {
        return;
    }

    /**
     *
     * Methods called from the C++ side
     *
     */
    public static void startService()
    {
        mBillingManager.startService();

    }

    public static boolean isInitialized() {
        return mBillingManager.isServiceConnected();
    }

    public static void purchase(final String sku, boolean isConsumable)
    {
        //threadIncrementTaskCounter();

        mBillingManager.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                delegateOnTransactionStart();
                mBillingManager.initiatePurchaseFlow(sku, BillingClient.SkuType.INAPP );
            }
        });
    }
    public static void restorePurchases()
    {
        mBillingManager.queryPurchases();
    }

    public static void addItemDataRequest(String productId, boolean isConsumable)
    {
        instance.pendingItemData.put(productId, isConsumable);
    }

    public static void shutdown()
    {
        Log.e(TAG, "shutdown");
    }

    public static void startProductDataRequest()
    {
        final List<String> moreSkus = new ArrayList<String>(instance.pendingItemData.keySet());

        mBillingManager.querySkuDetailsAsync(BillingClient.SkuType.INAPP, moreSkus, instance);
        instance.pendingItemData.clear();
    }

    /**
     *
     * Methods to call back into the C++ side
     *
     */

    public static native void onServiceStarted();
    public static native boolean onPurchaseSucceed(String productId);
    public static native void onPurchaseFail(int result);
    public static native void delegateOnTransactionStart();
    public static native void delegateOnTransactionEnd();
    public static native void onProductData(String productId, String name, String desc, String priceStr, float price);
    public static native void onProductRequestSuccess();
    public static native void onProductRequestFailed();
    public static native void onInitialized();

    /**
     *
     * Helper methods to integrate this lib into your app. Should be called in
     * your main activity - please read the docs provided with this library!
     *
     */

    public static void setPublicKey(final String publicKey)
    {
        mBillingManager.setKey(publicKey);
    }
}
