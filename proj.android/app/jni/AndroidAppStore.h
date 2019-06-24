#ifndef Pirateer_AndroidAppStore_h
#define Pirateer_AndroidAppStore_h

#include "external/AppStore.h"

class AndroidAppStore : public AppStore
{
public:
    virtual ~AndroidAppStore();

    bool init();

    bool startService();
    void shutdown();

    void purchase(Product* const product);
    bool isPurchaseReady() const;

    void restorePurchases();


};
#endif
