//
//  Fabric.hpp
//  Elixir
//
//  Created by Peter Respondek on 6/2/16.
//
//

#ifndef Fabric_hpp
#define Fabric_hpp

#include <string>

class CFabric
{
public:
    class CAnswers {
    public:
        static void logSignUpWithMethod ( const std::string& method,
                                         bool success );
        static void logLoginWithMethod ( const std::string& method,
                                         bool success );
        static void logInviteWithMethod ( const std::string& method );
        static void logPurchaseWithPrice ( float price,
                                           const std::string& currency,
                                           bool success,
                                           const std::string& itemName,
                                           const std::string& itemType,
                                           const std::string& itemId );
        static void logLevelStart ( const std::string& method );
        static void logLevelEnd ( const std::string& method,
                                  int score,
                                  bool success );
    private:
        CAnswers () {}
    };
private:
    CFabric () {}
};

#endif /* Fabric_hpp */
