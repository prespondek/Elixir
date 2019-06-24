#ifndef Pirateer_IOSAppStore_h
#define Pirateer_IOSAppStore_h

#include "../Classes/external/AppStore.h"

class IOSAppStore : public AppStore
{
public:
    virtual ~IOSAppStore();
    
    bool init();
    
    bool startService();
    void shutdown();
    
    void purchase(Product* const product);
    bool isPurchaseReady() const;
    
    void restorePurchases();
    
private:
    
};

#endif
