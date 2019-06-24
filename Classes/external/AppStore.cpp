//
//  AppStore.cpp
//  Clear for Action
//
//  Created by Peter Respondek on 9/4/14.
//
//

#include "../external/AppStore.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "IOSAppStore.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../../proj.android/app/jni/AndroidAppStore.h"
#endif

using std::string;
using std::make_pair;

static AppStore* g_apStore = NULL;

Product::Product(const char* const productId)
: price(0)
, localizedPrice("???")
, localizedName("Unavailable")
, localizedDescription("Unavailable")
, purchasedCounter(0)
, productId(std::string(productId))
{
}

Product::~Product()
{
}

std::string Product::getProductId() const
{
    return productId;
}

bool Product::canBePurchased() const
{
    AppStore* ap_store = AppStore::getInstance();
    if (!ap_store || !ap_store->isPurchaseReady()) {
        return false;
    }
    
    return true;
}

void Product::purchase()
{
    AppStore* ap_store = AppStore::getInstance();
    if (!ap_store) {
        CCAssert(false, "service has to be set");
        return;
    }
    
    ap_store->purchase(getProductId().c_str());
}

void Product::onHasBeenPurchased()
{
    ++purchasedCounter;
}

int Product::getPurchasedCounter()
{
    return purchasedCounter;
}

bool Product::hasBeenPurchased() const
{
    return (purchasedCounter > 0);
}

void Product::consume()
{
}

ProductConsumable::ProductConsumable(const char* const productId, const float quantityPerPurchase)
: Product(productId)
, quantityPerPurchase(quantityPerPurchase)
{
}

ProductConsumable::~ProductConsumable()
{
    CCAssert(getQuantity() == 0, "unused consumable quantity detected!");
}

void ProductConsumable::consume()
{
    Product::consume();
    purchasedCounter--;
}

float ProductConsumable::getQuantity() const
{
    return (purchasedCounter * quantityPerPurchase);
}

AppStore* AppStore::getInstance()
{
    if (!g_apStore) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        g_apStore = new (std::nothrow) IOSAppStore();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        g_apStore = new (std::nothrow) AndroidAppStore();
#endif
        if (g_apStore)
            g_apStore->init();
        else {
            delete g_apStore;
            g_apStore = nullptr;
        }
    }

    return g_apStore;
}


void AppStore::purgeAppStore()
{
    delete g_apStore;
    g_apStore = nullptr;
}

AppStore::AppStore()
: ignoreUnusedConsumableQuantities(false)
, _delegates()
, _state(AppStoreStateType::STOPPED)
, _products()
, productIdAliases()
{
}

bool AppStore::isInitialized() const
{
    return _state == STARTED;
}

AppStore::~AppStore()
{
    for (auto& pair : _products) {
        auto& product = pair.second;
        
        if (ignoreUnusedConsumableQuantities) {
            auto consumable = dynamic_cast<ProductConsumable* const>(product);
            if (consumable) {
                consumable->consume();
            }
        }
        
        delete product;
    }
    
    _products.clear();
    productIdAliases.clear();
}

void AppStore::purchase(Product* const product)
{
    _state = PURCHASEING;
}

void AppStore::addProduct(Product* const product)
{
    if (!product) {
        CCAssert(false, "product must be given");
        return;
    }
    CCAssert(!hasProduct(product->getProductId().c_str()), "productId already in use");
    
    _products.insert(make_pair(
                               product->getProductId(),
                               product
                               ));
}

void AppStore::addProduct(Product* const product, const char* const alias)
{
    CCAssert(!hasProduct(alias), "given alias already in use");
    
    addProduct(product);
    productIdAliases.insert(make_pair(
                                      string(alias),
                                      product->getProductId()
                                      ));
}

const ProductList& AppStore::getProducts() const
{
    return _products;
}

Product* AppStore::getProduct(const char* const productIdOrAlias) const
{
    auto productId = string(productIdOrAlias);
    auto product = _products.find(productId);
    if (product != _products.end()) {
        return product->second;
    }
    
    if (productIdAliases.count(productId) > 0) {
        auto aliasedId = productIdAliases.at(productId);
        auto aliasedProduct = _products.find(aliasedId);
        if (aliasedProduct != _products.end()) {
            return aliasedProduct->second;
        }
    }
    
    CCAssert(false, "invalid productId or alias given");
    return NULL;
}

void AppStore::reachabilityStatusChanged( ReachabilityStatus status )
{
    if (!_delegates.size()) return;
    if (status == NetNotReachable && _state != STOPPED && _state != PURCHASEING) {
        stopService();
    }
    else if (status != NetNotReachable && getState() == STOPPED) {
        startService();
    }
    
}

ProductConsumable* AppStore::getProductConsumable(const char* const productIdOrAlias) const
{
    return dynamic_cast<ProductConsumable* const>(getProduct(productIdOrAlias));
}

bool AppStore::hasProduct(const char* const productIdOrAlias) const
{
    auto productId = string(productIdOrAlias);
    
    if (_products.count(productId) > 0) {
        return true;
    }
    
    if (productIdAliases.count(productId) == 0) {
        return false;
    }
    
    auto aliasId = productIdAliases.at(productId);
    return (_products.count(aliasId) > 0);
}

void AppStore::addDelegate(AppStoreDelegate* delegate)
{
    if (delegate) {
        _delegates.push_back(delegate);
    }
}

void AppStore::removeDelegate(AppStoreDelegate* delegate)
{
    auto iter = std::find(_delegates.begin(),_delegates.end(), delegate );
    if ( iter != _delegates.end() ) {
        _delegates.erase(iter);
    }
}

void AppStore::purchase(const char* const productIdOrAlias)
{
    if (!isPurchaseReady()) {
        CCAssert(false, "backend service not started yet");
        return;
    }
    
    auto product = getProduct(productIdOrAlias);
    if (product) {
        purchase(product);
    }
}

bool AppStore::startService()
{
    if (getState() != STOPPED) {
        //CCAssert( false, "service already started" );
        return false;
    }
    _state = STARTING;
    return true;
}

void AppStore::productRequestSuccess()
{
    _state = STARTED;
    checkProducts();
    
    onAppStoreStarted();
}

void AppStore::checkProducts()
{
    if (_state != STARTED) return;
    for ( auto& i : _products ) {
        if (i.second->getPurchasedCounter() > 0) {
            onPurchaseSucceed(i.second);
        }
    }
    
}

void AppStore::transactionRequestSuccess()
{
    onTransactionEnd();
    if (_state == PURCHASEING) {
        _state = STARTED;
    }
}

void AppStore::transactionRequestFailed()
{
    onTransactionEnd();
    stopService();
}


void AppStore::productRequestFailed()
{
    stopService();
}

void AppStore::stopService()
{
    _state = STOPPED;
    onAppStoreStopped();
    shutdown();
}

AppStore::AppStoreStateType AppStore::getState() const
{
    return _state;
}

bool AppStore::isPurchaseReady() const
{
    return (_delegates.size() && isInitialized());
}

void AppStore::restorePurchases()
{
    if (!isPurchaseReady()) {
        CCAssert(false, "backend service not started yet");
        return;
    }
}

void AppStore::onAppStoreStarted() {
    for ( auto delegate : _delegates ) {
        delegate->onAppStoreStarted();
    }
}

void AppStore::onAppStoreStopped() {
    for ( auto delegate : _delegates ) {
        delegate->onAppStoreStopped();
    }
}

void AppStore::onPurchaseSucceed(Product* const product) {
    for ( auto delegate : _delegates ) {
        delegate->onPurchaseSucceed(product);
    }
}

void AppStore::onPurchaseFail( const std::string& reason ) {
    for ( auto delegate : _delegates ) {
        delegate->onPurchaseFail( reason );
    }
}

void AppStore::onPurchaseCancelled() {
    for ( auto delegate : _delegates ) {
        delegate->onPurchaseCancelled();
    }
}

void AppStore::onTransactionStart() {
    for ( auto delegate : _delegates ) {
        delegate->onTransactionStart();
    }
}

void AppStore::onTransactionEnd() {
    for ( auto delegate : _delegates ) {
        delegate->onTransactionEnd();
    }
}

void AppStore::onRestoreSucceed() {
    for ( auto delegate : _delegates ) {
        delegate->onRestoreSucceed();
    }
}

void AppStore::onRestoreFail() {
    for ( auto delegate : _delegates ) {
        delegate->onRestoreFail();
    }
}
