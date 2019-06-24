//
//  IOSFabric.m
//  Elixir
//
//  Created by Peter Respondek on 6/2/16.
//
//

#include "../Classes/external/Fabric.h"
#import <Crashlytics/Answers.h>


void CFabric::CAnswers::logSignUpWithMethod( const std::string& method, bool success )
{
    [Answers logSignUpWithMethod:[NSString stringWithUTF8String: method.c_str()]
                        success:[NSNumber numberWithBool:success]
               customAttributes:@{}];
}

void CFabric::CAnswers::logLoginWithMethod ( const std::string& method, bool success )
{
    [Answers logLoginWithMethod:[NSString stringWithUTF8String: method.c_str()]
                        success:[NSNumber numberWithBool:success]
               customAttributes:@{}];
}
void CFabric::CAnswers::logInviteWithMethod ( const std::string& method )
{
    [Answers logInviteWithMethod:[NSString stringWithUTF8String: method.c_str()]
                customAttributes:@{}];
}
void CFabric::CAnswers::logPurchaseWithPrice ( float price,
                                  const std::string& currency,
                                  bool success,
                                  const std::string& itemName,
                                  const std::string& itemType,
                                  const std::string& itemId )
{
    [Answers logPurchaseWithPrice:[NSDecimalNumber decimalNumberWithString:[NSString stringWithUTF8String: std::to_string(price).c_str()]]
                         currency:[NSString stringWithUTF8String: currency.c_str()]
                          success:[NSNumber numberWithBool:success]
                         itemName:[NSString stringWithUTF8String: itemName.c_str()]
                         itemType:[NSString stringWithUTF8String: itemType.c_str()]
                           itemId:[NSString stringWithUTF8String: itemId.c_str()]
                 customAttributes:@{}];
}
void CFabric::CAnswers::logLevelStart ( const std::string& method )
{
    [Answers logLevelStart:[NSString stringWithUTF8String: method.c_str()]
          customAttributes:@{}];
}
void CFabric::CAnswers::logLevelEnd ( const std::string& method, int score, bool success )
{
    [Answers logLevelEnd:[NSString stringWithUTF8String: method.c_str()]
                   score:[NSNumber numberWithInt:score]
                 success:[NSNumber numberWithBool:success]
        customAttributes:@{}];
}
