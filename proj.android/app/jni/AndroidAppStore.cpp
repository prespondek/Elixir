#include <jni.h>
#include "cocos2d.h"
#include "platform/android/jni/JniHelper.h"
#include "AndroidAppStore.h"

const char* const CLASS_NAME = "com/bridge/appstore/AndroidAppStore";

/**
 * Java -->> C++
 */

extern "C" {

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_onServiceStarted(JNIEnv* env, jclass clazz)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "app_store should be already set");
    if (app_store) {
        for (auto& row : app_store->getProducts()) {
            bool isConsumable = (dynamic_cast<ProductConsumable* const>(row.second) != NULL);
            std::string productId = row.second->getProductId();
            cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME, "addItemDataRequest",
                                                     productId, isConsumable);
        }
    }
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME,"startProductDataRequest");
}

JNIEXPORT jboolean JNICALL Java_com_bridge_appstore_AndroidAppStore_onPurchaseSucceed(JNIEnv* env, jclass clazz, jstring jProductId)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "app_store should be already set");

    if (!app_store) {
        return false;
    }

    std::string productId = cocos2d::JniHelper::jstring2string(jProductId);
    auto product = app_store->getProduct(productId.c_str());

    if (!product) {
        return false;
    }

    product->onHasBeenPurchased();
    app_store->onPurchaseSucceed(product);
    return true;
}

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_onPurchaseFail(JNIEnv* env, jclass clazz, jint jerror)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "app_store should be already set");
    std::string error;
    bool reportFailure = true;

    switch (jerror) {
        /** User pressed back or canceled a dialog */
        case 1: // USER_CANCELED = 1;
            error =  "User cancelled.";
        case -1: // SERVICE_DISCONNECTED
            error =  "Service Disconnected.";
            /** Network connection is down */
        case 2: // SERVICE_UNAVAILABLE = 2;
            error =  "No network connect.";
            /** Billing API version is not supported for the type requested */
        case 3: // BILLING_UNAVAILABLE = 3;
            error =  "Billing unavailable.";
            /** Requested product is not available for purchase */
        case 4: // ITEM_UNAVAILABLE = 4;
            error =  "Product unavailable.";
            /** Failure to consume since item is not owned */
        case 7: // ITEM_ALREADY_OWNED = 7;
            reportFailure = false;
            app_store->restorePurchases();
            error =  "Product already owned.";
            /**
             * Invalid arguments provided to the API. This error can also indicate that the application was
             * not correctly signed or properly set up for In-app Billing in Google Play, or does not have
             * the necessary permissions in its manifest
             */
        case 5: // DEVELOPER_ERROR = 5;
            /** Fatal error during the API action */
        case 6: // ERROR = 6;
            /** Failure to purchase since item is already owned */
        case 8: // ITEM_NOT_OWNED = 8;
        default:
            error =  "Unkown error occured";
    }

    if (reportFailure) {
        app_store->transactionRequestFailed();
        app_store->onPurchaseFail(error);
    } else {
        app_store->transactionRequestSuccess();
        app_store->onPurchaseCancelled();
        app_store->onTransactionEnd();
    }
}

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_delegateOnTransactionStart(JNIEnv* env, jclass clazz)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "app_store should be already set");
    app_store->onTransactionStart();
}

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_delegateOnTransactionEnd(JNIEnv* env, jclass clazz)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "app_store should be already set");
    app_store->onTransactionEnd();
    app_store->transactionRequestSuccess();
}

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_onProductRequestSuccess(JNIEnv* env, jclass clazz)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "onProductRequestSuccess");
    app_store->productRequestSuccess();
}

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_onProductRequestFailed(JNIEnv* env, jclass clazz)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "onProductRequestFailed");
    app_store->productRequestFailed();
}

JNIEXPORT void JNICALL Java_com_bridge_appstore_AndroidAppStore_onProductData(JNIEnv* env, jclass clazz, jstring jProductId, jstring jName, jstring jDesc, jstring jPriceStr, jfloat jprice)
{
    AppStore* app_store = AppStore::getInstance();
    CCASSERT(app_store, "app_store should be already set");

    std::string productId = cocos2d::JniHelper::jstring2string(jProductId);
    auto product = app_store->getProduct(productId.c_str());

    if (!product) {
        return;
    }

    std::string localizedName = cocos2d::JniHelper::jstring2string(jName);
    std::string localizedDescription = cocos2d::JniHelper::jstring2string(jDesc);
    std::string localizedPrice = cocos2d::JniHelper::jstring2string(jPriceStr);

    product->localizedName = localizedName;
    product->localizedDescription = localizedDescription;
    product->localizedPrice = localizedPrice;
    product->price = (float)jprice;
}


} // extern "C"

/**
 * Public API
 */

/*IOSAppStore::IOSAppStore(Manager& manager)
: manager(manager)
{
    backend::helper::globalManager = &manager;
}*/

AndroidAppStore::~AndroidAppStore()
{
    shutdown();
    //backend::helper::globalManager = NULL;
}

void AndroidAppStore::shutdown()
{
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME,"shutdown");
}

bool AndroidAppStore::init()
{
    return true;
}

bool AndroidAppStore::startService()
{
    if (!AppStore::startService()) return false;

    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME,"startService");
    return true;
}

bool AndroidAppStore::isPurchaseReady() const
{
    return cocos2d::JniHelper::callStaticBooleanMethod(CLASS_NAME,"isInitialized");
}

void AndroidAppStore::restorePurchases()
{
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME,"restorePurchases");
}


void AndroidAppStore::purchase(Product* const product)
{
    bool isConsumable = (dynamic_cast<ProductConsumable* const>(product) != NULL);
    AppStore::purchase(product);
    cocos2d::JniHelper::callStaticVoidMethod(CLASS_NAME,"purchase", product->getProductId().c_str(), isConsumable);
}
