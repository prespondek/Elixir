//
//  AppStore.h
//  Clear for Action
//
//  Created by Peter Respondek on 9/4/14.
//
//

#include "cocos2d.h"
#include "Reachability.h"
USING_NS_CC;

#ifndef __Clear_for_Action__AppStore__
#define __Clear_for_Action__AppStore__

class Product
{
public:
    float           price;
    std::string     localizedPrice;
    std::string     localizedName;
    std::string     localizedDescription;
    LanguageType    locale;
    
    explicit Product(const char* const productId);
    virtual ~Product();
    
    std::string getProductId() const;
    
    bool canBePurchased() const;
    void purchase();
    
    void onHasBeenPurchased();
    bool hasBeenPurchased() const;
    virtual void consume();
    int getPurchasedCounter();
    
protected:
    int purchasedCounter;
    
private:
    const std::string productId;
};

class ProductConsumable : public Product
{
public:
    ProductConsumable(const char* const productId, const float quantityPerPurchase);
    virtual ~ProductConsumable();
    
    virtual void consume() override;
    float getQuantity() const;
    
protected:
    const float quantityPerPurchase;
};

typedef std::map<const std::string, Product* const> ProductList;

class AppStoreDelegate
{
public:
    virtual void onAppStoreStarted()     {};
    virtual void onAppStoreStopped()     {};
    
    virtual void onPurchaseSucceed(Product* const product) = 0;
    virtual void onPurchaseFail(const std::string& reason) {};
    virtual void onPurchaseCancelled()   {};
    
    virtual void onTransactionStart()    {};
    virtual void onTransactionEnd()      {};
    
    virtual void onRestoreSucceed()      {};
    virtual void onRestoreFail()         {};
    
};

class AppStore : public ReachabilityDelegate
{
public:
    
    enum AppStoreStateType {
        STOPPED,
        STARTED,
        STARTING,
        PURCHASEING,
    };
    
    static AppStore* getInstance( );
    
    void purgeAppStore();
    
    virtual bool init ( ) = 0;
    virtual bool isInitialized () const;
    
    bool ignoreUnusedConsumableQuantities;
    
    void reachabilityStatusChanged( ReachabilityStatus status );
    
    void addProduct(Product* const product);
    void addProduct(Product* const product, const char* const alias);
    void checkProducts();
    
    const ProductList& getProducts() const;
    Product* getProduct(const char* const productIdOrAlias) const;
    ProductConsumable* getProductConsumable(const char* const productIdOrAlias) const;
    bool hasProduct(const char* const productIdOrAlias) const;
    
    void purchase(const char* const productIdOrAlias);
    virtual bool isPurchaseReady() const;
    
    AppStoreStateType getState() const;
    
    void stopService();
    virtual bool startService() = 0;
    virtual void purchase(Product* const product) = 0;
    virtual void shutdown() = 0;
    virtual void restorePurchases() = 0;
    
    // Callback recievers
    virtual void productRequestFailed();
    virtual void productRequestSuccess();
    virtual void transactionRequestSuccess();
    virtual void transactionRequestFailed();
    
    void addDelegate(AppStoreDelegate* delegate);
    void removeDelegate(AppStoreDelegate* delegate);
    
    void onAppStoreStarted();
    void onAppStoreStopped();
    
    void onPurchaseSucceed(Product* const product);
    void onPurchaseFail( const std::string& reason );
    void onPurchaseCancelled();
    
    void onTransactionStart();
    void onTransactionEnd();
    
    void onRestoreSucceed();
    void onRestoreFail();
    
protected:
    AppStore();
    virtual ~AppStore();
    
    ProductList _products;
    std::map<std::string, std::string> productIdAliases;
private:
    
    std::vector<AppStoreDelegate*> _delegates;
    AppStoreStateType _state;
    
};
#endif /* defined(__Clear_for_Action__AppStore__) */
