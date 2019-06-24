#include "IOSAppStore.h"

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>


@interface BackendIos : NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>
{
@public bool initialized;
@public AppStore* manager;
@public int transactionDepth;
}

+ (BackendIos*)shared;

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error;

#pragma mark -
#pragma mark SKProductsRequestDelegate

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response;

#pragma mark -
#pragma mark SKPaymentTransactionObserver

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions;
- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue;
- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error;

#pragma mark -
#pragma mark Helper

- (void)failedTransaction:(SKPaymentTransaction *)transaction;
- (void)completeTransaction:(SKPaymentTransaction *)transaction;
- (void)restoreTransaction:(SKPaymentTransaction *)transaction;

@end

static BackendIos* instance = nil;

@implementation BackendIos


#pragma mark -
#pragma mark SKProductsRequestDelegate

+ (BackendIos*)shared
{
    @synchronized(self) {
        if (instance == nil) {
            instance = [[self alloc] init];
            instance->initialized = false;
        }
    }
    return instance;
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    if (manager) manager->productRequestFailed();
}

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    if (!manager || manager->getState() != AppStore::STARTING) {
        return;
    }
    
    NSNumberFormatter* numberFormatter = [[[NSNumberFormatter alloc] init] autorelease];
    [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
    
    for (SKProduct* skProduct in response.products) {
        const char* productId = [[skProduct productIdentifier] cStringUsingEncoding:NSASCIIStringEncoding];
        Product* avProduct = manager->getProduct(productId);
        if (avProduct == NULL) {
            NSLog(@"[Payment] productsRequest: Product not found on our side - productId: %s", productId);
            continue;
        }
        
        avProduct->price = [[skProduct price] floatValue];
        [numberFormatter setLocale:skProduct.priceLocale];
        avProduct->localizedPrice = [[numberFormatter stringFromNumber:skProduct.price] UTF8String];
        
        NSString* localizedName = [skProduct localizedTitle];
        if (localizedName != NULL) {
            avProduct->localizedName = [localizedName UTF8String];
        }
        NSLocale* locale = [skProduct priceLocale];
        if (locale != NULL) {
            avProduct->locale = utils::getLanguageTypeByISO2([locale.languageCode UTF8String]);
        }
        NSString* localizedDescription = [skProduct localizedDescription];
        if (localizedDescription != NULL) {
            avProduct->localizedDescription = [localizedDescription UTF8String];
        }
    }
    
    for (NSString* productId in response.invalidProductIdentifiers) {
        NSLog(@"[Payment] productsRequest: Product not found on apple side - productId: %@", productId);
    }
    
    manager->productRequestSuccess();
}

#pragma mark -
#pragma mark SKPaymentTransactionObserver

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
    for (SKPaymentTransaction *transaction in transactions) {
        switch (transaction.transactionState) {
            case SKPaymentTransactionStatePurchased:
                [self completeTransaction:transaction];
                break;
                
            case SKPaymentTransactionStateFailed:
                [self failedTransaction:transaction];
                break;
                
            case SKPaymentTransactionStateRestored:
                [self completeTransaction:transaction];
                break;
                
            case SKPaymentTransactionStatePurchasing:
                // this state is known, fine and can be ignored
                break;
                
            default:
                NSLog(@"[Payment] paymentQueue: UNHANDELED: %ld", (long)transaction.transactionState);
                break;
        }
    }
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
    if (manager) {
        manager->onRestoreSucceed();
    }
    
    transactionDepth = std::max(0, --transactionDepth);
    if (manager && transactionDepth == 0) {
        manager->onTransactionEnd();
    }
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
    if (error.code != SKErrorPaymentCancelled) {
        NSLog(@"[Payment] restoreCompletedTransactions failed: %@", error.localizedDescription);
        
        if (manager) {
            manager->onRestoreFail();
        }
    }
    
    transactionDepth = std::max(0, --transactionDepth);
    if (manager && transactionDepth == 0) {
        manager->onTransactionEnd();
    }
}

#pragma mark -
#pragma mark Helper

- (void)completeTransaction:(SKPaymentTransaction *)transaction
{
    // it's important to NOT CALL finishTransaction in this case! because we
    // were unable to process this transaction in the user application. apple
    // will try to deliver this transaction again.
    if (!manager) {
        transactionDepth = std::max(0, --transactionDepth);
        NSLog(@"[Payment] completeTransaction failed: no manager set");
        return;
    }
    
    const char* productId = [transaction.payment.productIdentifier cStringUsingEncoding:NSASCIIStringEncoding];
    Product* product = manager->getProduct(productId);
    if (product) {
        product->onHasBeenPurchased();
    } else {
        NSLog(@"[Payment] completeTransaction failed: invalid productId: %s", productId);
    }
    
    // roughly the same reason as the return above. this is a real transaction and
    // the product was valid a few moments ago. seems to be a bug in the
    // application? nevermind. it's important to try this transaction again
    // and NOT to discard it!
    if (product) {
        [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
        
        if (manager && manager->getState() != AppStore::STARTING) {
            manager->onPurchaseSucceed(product);
        }
    }
    
    transactionDepth = std::max(0, --transactionDepth);
    if (manager && transactionDepth == 0) {
        manager->transactionRequestSuccess();
    }
}

- (void)restoreTransaction:(SKPaymentTransaction *)transaction
{
    // we can return early in this case. and we're allowed to finish this
    // transaction too. why? because it's just a restoreTransaction and not
    // a important purchase transaction that must reach the users application
    // as in completeTransaction().
    if (!manager) {
        transactionDepth = std::max(0, --transactionDepth);
        NSLog(@"[Payment] completeTransaction failed: no manager set");
        return;
    }
    
    const char* productId = [transaction.originalTransaction.payment.productIdentifier cStringUsingEncoding:NSASCIIStringEncoding];
    Product* product = manager->getProduct(productId);
    if (product) {
        product->onHasBeenPurchased();
    } else {
        NSLog(@"[Payment] restoreTransaction invalid productId: %s", productId);
    }
    
    if (manager && product) {
        manager->onPurchaseSucceed(product);
    }
    
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
    
    transactionDepth = std::max(0, --transactionDepth);
    if (manager && transactionDepth == 0) {
        manager->transactionRequestSuccess();
    }
}

- (void)failedTransaction:(SKPaymentTransaction *)transaction
{
    bool reportFailure = true;
    switch (transaction.error.code) {
        case SKErrorPaymentCancelled:
            NSLog(@"[Payment] failedTransaction: SKErrorPaymentCancelled");
            reportFailure = false;
            break;
            
        case SKErrorUnknown:
            NSLog(@"[Payment] failedTransaction: SKErrorUnknown: %@ | %@",
                  transaction.error.localizedDescription,
                  transaction.error.localizedFailureReason );
            break;
            
        case SKErrorClientInvalid:
            NSLog(@"[Payment] failedTransaction: SKErrorClientInvalid");
            break;
            
        case SKErrorPaymentInvalid:
            NSLog(@"[Payment] failedTransaction: SKErrorPaymentInvalid");
            break;
            
        case SKErrorPaymentNotAllowed:
            NSLog(@"[Payment] failedTransaction: SKErrorPaymentNotAllowed");
            break;
            
        default:
            NSLog(@"[Payment] failedTransaction: UNHANDELED: %ld", (long)transaction.error.code);
            break;
    }
    if (manager && !reportFailure) {
        manager->transactionRequestSuccess();
        manager->onPurchaseCancelled();
    }
    
    if (manager && reportFailure) {
        manager->transactionRequestFailed();
        manager->onPurchaseFail(transaction.error.localizedDescription.UTF8String);
    }
    
    transactionDepth = std::max(0, --transactionDepth);
    if (manager && transactionDepth == 0) {
        manager->onTransactionEnd();
    }
    
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
}

@end


BackendIos* const __getIosBackend()
{
    return [BackendIos shared];
}

IOSAppStore::~IOSAppStore()
{
    shutdown();
}

bool IOSAppStore::startService()
{
    if (!AppStore::startService()) return false;
    // configure BackendIos
    __getIosBackend()->initialized = true;
    __getIosBackend()->manager = this;
    __getIosBackend()->transactionDepth = 0;
    
    // register transcationObserver
    [[SKPaymentQueue defaultQueue] addTransactionObserver:__getIosBackend()];
    
    // convert Avalon::Payment::ProductList into NSMutableSet
    NSMutableSet* products = [[[NSMutableSet alloc] init] autorelease];
    for (const auto& pair : getProducts()) {
        const char* const productId = pair.second->getProductId().c_str();
        [products addObject:[NSString stringWithUTF8String:productId]];
    }
    
    // fetch product details
    SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:products];
    request.delegate = __getIosBackend();
    [request start];
    return true;
}

bool IOSAppStore::init()
{
    return true;
}

void IOSAppStore::shutdown()
{
    if (!isInitialized()) {
        return;
    }
    
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:__getIosBackend()];
    __getIosBackend()->initialized = false;
    __getIosBackend()->manager = NULL;
}

void IOSAppStore::purchase(Product* const product)
{
    AppStore::purchase(product);
    if (__getIosBackend()->transactionDepth == 0) {
        onTransactionStart();
    }
    __getIosBackend()->transactionDepth += 1;
    
    NSString* productId = [[[NSString alloc] initWithUTF8String:product->getProductId().c_str()] autorelease];
    
    SKPayment *payment = [SKPayment paymentWithProductIdentifier:productId];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

bool IOSAppStore::isPurchaseReady() const
{
    if (AppStore::isPurchaseReady())
        return [SKPaymentQueue canMakePayments];
    else return false;
}

void IOSAppStore::restorePurchases()
{
    if (__getIosBackend()->transactionDepth == 0) {
        onTransactionStart();
    }
    __getIosBackend()->transactionDepth += 1;
    
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}
