//
//  ElixirLocalization.cpp
//  Elixir
//
//  Created by Peter Respondek on 11/9/15.
//
//

#include "../data/ElixirLocalization.h"

bool ProfanityFilter::checkString ( std::string str )
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    for ( auto i : profanity_str ) {
        if ( str.find(i) != std::string::npos ) {
            return true;
        }
    }
    return false;
}


bool ProfanityFilter::init()
{
    LanguageType lang = Application::getInstance()->getCurrentLanguage();
    
    switch (lang) {
        default:
            profanity_str = {
                "4r5e", "5h1t", "5hit", "a55", "anal", "anus", "ar5e", "arrse", "arse", "ass", "ass-fucker","asses","assfucker", "assfukka" , "asshole", "assholes", "asswhole", "a_s_s", "b!tch", "b00bs", "b17ch", "b1tch", "ballbag", "balls", "ballsack", "bastard", "beastial", "beastiality", "bellend", "bestial", "bestiality", "bi+ch", "biatch", "bitch", "bitcher", "bitchers", "bitches", "bitchin", "bitching", "blood", "blow job", "blowjob", "blowjobs", "boiolas", "bollock", "bollok", "boner", "boob", "boobs", "booobs", "boooobs", "booooobs", "booooooobs", "breasts", "buceta", "bugger", "bum", "bunny fucker", "butt", "butthole", "buttmuch", "buttplug", "c0ck", "c0cksucker",
                "carpet muncher", "cawk", "chink", "cipa", "cl1t", "clit", "clitoris", "clits", "cnut", "cock", "cock-sucker",
                "cockface", "cockhead", "cockmunch", "cockmuncher", "cocks", "cocksuck", "cocksucked", "cocksucker",
                "cocksucking", "cocksucks", "cocksuka", "cocksukka", "cok", "cokmuncher", "coksucka", "coon", "cox", "crap",
                "cum", "cummer", "cumming", "cums", "cumshot", "cunilingus", "cunillingus", "cunnilingus", "cunt",
                "cuntlick","cuntlicker","cuntlicking","cunts","cyalis","cyberfuc","cyberfuck","cyberfucked","cyberfucker","cyberfuckers","cyberfucking","d1ck","damn","dick","dickhead","dildo","dildos","dink","dinks","dirsa","dlck","dog-fucker","doggin","dogging","donkeyribber","doosh","duche","dyke","ejaculate","ejaculated","ejaculates ","ejaculating ","ejaculatings","ejaculation","ejakulate","f u c k","f u c k e r","f4nny","fag","fagging","faggitt","faggot","faggs","fagot","fagots","fags","fanny","fannyflaps","fannyfucker","fanyy","fatass","fcuk","fcuker","fcuking","feck","fecker","felching","fellate","fellatio","fingerfuck ","fingerfucked ","fingerfucker ","fingerfuckers","fingerfucking ","fingerfucks ","fistfuck","fistfucked ","fistfucker ","fistfuckers ","fistfucking ","fistfuckings ","fistfucks ","flange","fook","fooker","fuck","fucka","fucked","fucker","fuckers","fuckhead","fuckheads","fuckin","fucking","fuckings","fuckingshitmotherfucker","fuckme ","fucks","fuckwhit","fuckwit","fudge packer","fudgepacker","fuk","fuker","fukker","fukkin","fuks","fukwhit","fukwit","fux","fux0r","f_u_c_k","gangbang","gangbanged ","gangbangs ","gaylord","gaysex","goatse","God","god-dam","god-damned","goddamn","goddamned","hardcoresex","hell","heshe","hoar","hoare","hoer","homo","hore","horniest","horny","hotsex","jack-off ","jackoff","jap","jerk-off","jism","jiz","jizm","jizz","kawk","knob","knobead","knobed","knobend","knobhead","knobjocky","knobjokey","kock","kondum","kondums","kum","kummer","kumming","kums","kunilingus","l3i+ch","l3itch","labia","lmfao","lust","lusting","m0f0","m0fo","m45terbate","ma5terb8","ma5terbate","masochist","master-bate","masterb8","masterbat*","masterbat3","masterbate","masterbation","masterbations","masturbate","mo-fo","mof0","mofo","mothafuck","mothafucka","mothafuckas","mothafuckaz","mothafucked ","mothafucker","mothafuckers","mothafuckin","mothafucking ","mothafuckings","mothafucks","mother fucker","motherfuck","motherfucked","motherfucker","motherfuckers","motherfuckin","motherfucking","motherfuckings","motherfuckka","motherfucks","muff","mutha","muthafecker","muthafuckker","muther","mutherfucker","n1gga","n1gger","nazi","nigg3r","nigg4h","nigga","niggah","niggas","niggaz","nigger","niggers ","nob","nobhead","nobjocky","nobjokey","numbnuts","nutsack","orgasim ","orgasims ","orgasm","orgasms ","p0rn","pawn","pecker","penis","penisfucker","phonesex","phuck","phuk","phuked","phuking","phukked","phukking","phuks","phuq","pigfucker","pimpis","piss","pissed","pisser","pissers","pisses ","pissflaps","pissin ","pissing","pissoff ","poop","porn","porno","pornography","pornos","prick","pricks ","pron","pube","pusse","pussi","pussies","pussy","pussys ","rectum","retard","rimjaw","rimming","s hit","s.o.b.","sadist","schlong","screwing","scroat","scrote","scrotum","semen","sex","sh!+","sh!t","sh1t","shag","shagger","shaggin","shagging","shemale","shi+","shit","shitdick","shite","shited","shitey","shitfuck","shitfull","shithead","shiting","shitings","shits","shitted","shitter","shitters","shitting","shittings","shitty","skank","slut","sluts","smegma","smut","snatch","son-of-a-bitch","spac","spunk","s_h_i_t","t1tt1e5","t1tties","teets","teez","testical","testicle","tit","titfuck","tits","titt","tittie5","tittiefucker","titties","tittyfuck","tittywank","titwank","tosser","turd","tw4t","twat","twathead","twatty","twunt","twunter","v14gra","v1gra","vagina","viagra","vulva","w00se","wang","wank","wanker","wanky","whoar","whore","willies","willy","xrated","xxx"
                
            };
            break;
            
    }
    return true;
}


ElixirLocalization::~ElixirLocalization()
{
    
}

static ElixirLocalization* s_LocalisedStrings = NULL;
ElixirLocalization* ElixirLocalization::getInstance( )
{
    if (!s_LocalisedStrings)
    {
        s_LocalisedStrings = new (std::nothrow) ElixirLocalization();
        s_LocalisedStrings->init();
    }
    
    return s_LocalisedStrings;
}

bool ElixirLocalization::init()
{
    return LocalizedStrings::init();
}

const char* ElixirLocalization::getLanguageFont               ( const char* font, LanguageType lang )
{
    if (strcmp("fonts/NOTMK.TTF", font) == 0) {
        switch (lang) {
            default:
                return "fonts/NOTMK2.ttf";
            case LanguageType::CHINESE:
                return "fonts/mini-jian-yihei.TTF";
            case LanguageType::KOREAN:
                return "fonts/JejuGothic-Regular.ttf";
            case LanguageType::JAPANESE:
                return "fonts/corp_round_v1.ttf";
        }
    }
    return "fonts/NOTMK2.ttf";
}

bool ElixirLocalization::getLocalisedFile( std::string& lang_dict_file )
{
    switch (_currentLanguageType) {
        case LanguageType::CHINESE:     lang_dict_file = "lang_zh.plist"; break;
        case LanguageType::ENGLISH:     lang_dict_file = "lang_en.plist"; break;
        case LanguageType::FRENCH:      lang_dict_file = "lang_fr.plist"; break;
        case LanguageType::ITALIAN:     lang_dict_file = "lang_it.plist"; break;
        case LanguageType::GERMAN:      lang_dict_file = "lang_de.plist"; break;
        case LanguageType::SPANISH:     lang_dict_file = "lang_es.plist"; break;
        case LanguageType::DUTCH:       lang_dict_file = "lang_nl.plist"; break;
        case LanguageType::RUSSIAN:     lang_dict_file = "lang_ru.plist"; break;
        case LanguageType::KOREAN:      lang_dict_file = "lang_ko.plist"; break;
        case LanguageType::JAPANESE:    lang_dict_file = "lang_ja.plist"; break;
        case LanguageType::HUNGARIAN:   lang_dict_file = "lang_hu.plist"; break;
        case LanguageType::PORTUGUESE:  lang_dict_file = "lang_pt.plist"; break;
        case LanguageType::ARABIC:      lang_dict_file = "lang_ar.plist"; break;
        case LanguageType::NORWEGIAN:   lang_dict_file = "lang_nb.plist"; break;
        case LanguageType::POLISH:      lang_dict_file = "lang_pl.plist"; break;
        case LanguageType::TURKISH:     lang_dict_file = "lang_tr.plist"; break;
        case LanguageType::UKRAINIAN:   lang_dict_file = "lang_uk.plist"; break;
        case LanguageType::ROMANIAN:    lang_dict_file = "lang_ro.plist"; break;
        case LanguageType::BULGARIAN:   lang_dict_file = "lang_bg.plist"; break;
        default:                        lang_dict_file = "lang_en.plist"; return false;
    }
    return true;
}
