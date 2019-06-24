//
//  ElixirLocalization.hpp
//  Elixir
//
//  Created by Peter Respondek on 11/9/15.
//
//

#ifndef ElixirLocalization_hpp
#define ElixirLocalization_hpp

#include "../cocos_extensions/Lanyard_Localisation.h"

#define LOCALIZE(__STRING__) \
ElixirLocalization::getInstance()->LocalizedStrings::getLocalizedString(__STRING__) \

#define LOCALFONT(__STRING__) \
ElixirLocalization::getInstance()->LocalizedStrings::getLanguageFont(__STRING__) \


class ElixirLocalization : public LocalizedStrings
{
public:
    virtual         ~ElixirLocalization               ( );
    static          ElixirLocalization* getInstance   ( );
    virtual const char* getLanguageFont               ( const char* font, LanguageType lang );
    
protected:
    virtual bool    getLocalisedFile                ( std::string& str );

    bool init ();
};

class ProfanityFilter : public Ref {
public:
    CREATE_FUNC(ProfanityFilter);
    bool checkString ( std::string str );
    
protected:
    bool init ();
    
    std::vector<std::string> profanity_str;
};

#endif /* ElixirLocalization_hpp */
