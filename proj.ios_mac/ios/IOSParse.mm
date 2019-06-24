//
//  IOSParse.cpp
//  Elixir
//
//  Created by Peter Respondek on 9/16/15.
//
//


#include "IOSParse.h"
#import <Parse/Parse.h>
#import <ParseFacebookUtilsV4/PFFacebookUtils.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>


IOSParseWrapper::IOSParseWrapper()
{
    
}

bool IOSParseWrapper::isLoggedIn()
{
    PFUser* user = [PFUser currentUser];
    
    if ([PFFacebookUtils isLinkedWithUser:user]) return true;
    return false;
}

int IOSParseWrapper::fetchUser( )
{
    int query_id = incrementQueryId();
    PFUser* user = [PFUser currentUser];
    [user fetchInBackgroundWithBlock:^(PFObject * _Nullable object, NSError * _Nullable error) {
        NSArray* results = @[object];
        queryCallback(results, error, object.parseClassName.UTF8String, query_id);
    }];
    return query_id;
}

void IOSParseWrapper::loginWithFacebook ()
{
    FBSDKLoginManager *login = [[FBSDKLoginManager alloc] init];
    [login logOut];

    UIViewController* root_view_controller = [UIApplication sharedApplication].keyWindow.rootViewController;
    [login
     logInWithReadPermissions: @[@"public_profile", @"email", @"user_friends"]
     fromViewController: root_view_controller
     handler:^(FBSDKLoginManagerLoginResult *result, NSError *error) {
         if (error) {
             for ( auto i : _delegates) {
                 i->parseLoginCallback(ParseDelegate::BAD_SERVER, false);
             }
         } else if (result.isCancelled) {
             for ( auto i : _delegates) {
                 i->parseLoginCallback(ParseDelegate::CANCELLED, false);
             }
         } else {
             [PFFacebookUtils logInInBackgroundWithAccessToken:result.token block:^(PFUser *user, NSError *error) {
                 if (!user) {
                     for ( auto i : _delegates) {
                         i->parseLoginCallback(ParseDelegate::CANCELLED, false);
                     }
                 } else {
                     for ( auto i : _delegates) {
                         i->parseLoginCallback(ParseDelegate::SUCCESS, user.isNew);
                     }
                 }
             }];
         }
     }];
}

void IOSParseWrapper::logOut()
{
    [[PFFacebookUtils facebookLoginManager] logOut];
    [FBSDKAccessToken setCurrentAccessToken:nil];
    [PFUser logOut];
    for ( auto i : _delegates) {
        i->parseLogOutCallback();
    }
}

void IOSParseWrapper::queryCallback       ( void* p_result, void* p_error, const std::string& table_name, int query_id )
{
    NSArray *results = (NSArray*) p_result;
    NSError *error = (NSError*) p_error;
    for ( auto i : _delegates ) {
        i->parseQueryStarted(query_id,!error);
        for (PFObject* obj in results) {
            std::shared_ptr<SaveClass> cpfobj = i->parseQueryCallback( query_id );
            if (cpfobj) {
                std::shared_ptr<ParseInterfaceObject> pobj (
                    new ParseInterfaceObject (cpfobj, std::shared_ptr<IOSParseWrapperObject> (new IOSParseWrapperObject(obj))));
                pobj->loadObject();
                i->parseQueryResult( pobj, query_id );
            }
        }
        i->parseQueryComplete(query_id,!error);
    }
}


int IOSParseWrapper::findObjectWithKey ( const std::string& table, const std::string& key, const std::string& value, bool fromDataStore )
{
    int query_id = incrementQueryId();
    std::string table_name = table;
    
    //[[ParseIOS shared] setObject:nil];
    PFQuery *query = [PFQuery queryWithClassName: [NSString stringWithUTF8String:table.c_str()]];
    if (fromDataStore) [query fromLocalDatastore];
    [query whereKey:[NSString stringWithUTF8String:key.c_str()] equalTo:[NSString stringWithUTF8String:value.c_str()]];
    [query findObjectsInBackgroundWithBlock:^(NSArray *results, NSError *error) {
        queryCallback(results, error, table_name, query_id);
    }];
    return query_id;
}

int IOSParseWrapper::saveWithObjectId(const std::shared_ptr<SaveClass>& obj, const std::string &key)
{
    NSString* class_name = [NSString stringWithUTF8String:obj->getKey().c_str()];
    NSString* object_id = [NSString stringWithUTF8String:key.c_str()];

    int query_id = incrementQueryId();
    PFObject* pfobj = [PFObject objectWithoutDataWithClassName:class_name objectId:object_id];
    std::shared_ptr<ParseInterfaceObject> pobj (new ParseInterfaceObject (obj, std::shared_ptr<IOSParseWrapperObject> (new IOSParseWrapperObject(pfobj))));
    pobj->saveObject();
    [pfobj saveInBackgroundWithBlock:^(BOOL succeeded1, NSError* error) {
        [pfobj pinInBackgroundWithBlock:^(BOOL succeeded2, NSError* error) {
            for ( auto i : _delegates) {
                i->parseObjectSaved( pobj, query_id, succeeded1 );
            }
        }];
    }];
    return query_id;
}

void IOSParseWrapper::unpinWithName(const std::string &key)
{
    [PFObject unpinAllObjectsWithName:[NSString stringWithUTF8String:key.c_str()]];
}

int IOSParseWrapper::saveNewObject(const std::shared_ptr<SaveClass>& obj)
{
    int query_id = incrementQueryId();
    NSString* class_name = [NSString stringWithUTF8String:obj->getKey().c_str()];
    PFObject* pfobj = [PFObject objectWithClassName:class_name];
    std::shared_ptr<ParseInterfaceObject> pobj (new ParseInterfaceObject (obj, std::shared_ptr<IOSParseWrapperObject> (new IOSParseWrapperObject(pfobj))));
    pobj->saveObject();
    //_objs.push_back(pobj);
    [pfobj saveInBackgroundWithBlock:^(BOOL succeeded, NSError * _Nullable error) {
        for ( auto i : _delegates) {
            i->parseObjectSaved(pobj, query_id, succeeded );
        }
    }];
    return query_id;
}

void IOSParseWrapper::deleteWithObjectId ( const std::string& table, const std::string& key )
{
    NSString* class_name = [NSString stringWithUTF8String:table.c_str()];
    NSString* object_id = [NSString stringWithUTF8String:key.c_str()];
    
    PFObject* pfobj = [PFObject objectWithoutDataWithClassName:class_name objectId:object_id];
    [pfobj deleteEventually];
}

void IOSParseWrapper::loadWithObjectId ( const std::shared_ptr<SaveClass>& obj, const std::string &key )
{
    NSString* class_name = [NSString stringWithUTF8String:obj->getKey().c_str()];
    NSString* object_id = [NSString stringWithUTF8String:key.c_str()];
    NSError** error;
    
    PFObject* pfobj = [PFObject objectWithoutDataWithClassName:class_name objectId:object_id];
    [pfobj fetchFromLocalDatastore:error];
    if (error) {
        return;
    }
    ParseInterfaceObject pobj (obj, std::shared_ptr<IOSParseWrapperObject> (new IOSParseWrapperObject(pfobj)));
    pobj.loadObject();

}

int IOSParseWrapper::saveUser ( const std::shared_ptr<SaveClass>& obj, bool cloud_save )
{
    PFUser* user = [PFUser currentUser];
   std::shared_ptr<ParseInterfaceObject> pobj (new ParseInterfaceObject (obj, std::shared_ptr<IOSParseWrapperObject> (new IOSParseWrapperObject(user))));
    pobj->saveObject();
    
    int query_id = 0;
    
    if (cloud_save) {
        //_objs.push_back(pobj);
        query_id = incrementQueryId();
        [user saveInBackgroundWithBlock:^(BOOL succeeded, NSError * _Nullable error) {
            for ( auto i : _delegates) {
                i->parseObjectSaved( pobj, query_id, succeeded );
            }
        }];
    }
    return query_id;
}

std::shared_ptr<ParseInterfaceObject> IOSParseWrapper::loadUser ( const std::shared_ptr<SaveClass>& obj )
{
    PFUser* user = [PFUser currentUser];
    std::shared_ptr<ParseInterfaceObject> pobj (new ParseInterfaceObject (obj, std::shared_ptr<IOSParseWrapperObject> (new IOSParseWrapperObject(user))));
    pobj->loadObject();
    return pobj;
}

int IOSParseWrapper::getFriends( const std::vector<std::string>& friend_indices, bool fromDatastore )
{
    int query_id = incrementQueryId();
    
    NSMutableArray* fb_ids = [NSMutableArray array];
    for (auto& i : friend_indices) {
        [fb_ids addObject:[NSString stringWithUTF8String:i.c_str()]];
    }
    
    PFQuery* fb_query = [PFQuery queryWithClassName:@"Profile"];
    PFQuery* parse_query = [PFQuery queryWithClassName:@"Profile"];
    [parse_query whereKey:@"UUID" containedIn:fb_ids];
    [fb_query whereKey:@"FacebookId" containedIn:fb_ids];
    PFQuery* query = [PFQuery orQueryWithSubqueries:@[fb_query,parse_query]];
    [query orderByAscending:@"updatedAt"];

    [query findObjectsInBackgroundWithBlock:^(NSArray *results, NSError *error) {
        queryCallback(results, error, "Profile", query_id);
    }];
    return query_id;
}

int IOSParseWrapper::getMapScores( const std::string& obj_id,
                                  uint16_t language,
                                  uint16_t min,
                                  uint16_t max,
                                  bool fromDatastore )
{
    int query_id = incrementQueryId();
    NSNumber* nsmin = [NSNumber numberWithUnsignedShort:min];
    
    PFQuery* query = [PFQuery queryWithClassName:@"Profile"];
    [query whereKey:@"MaxLevel" lessThanOrEqualTo:      [NSNumber numberWithUnsignedShort:max]];
    [query whereKey:@"Level"    greaterThanOrEqualTo:   [NSNumber numberWithUnsignedShort:min]];
    [query whereKey:@"Language" equalTo:                [NSNumber numberWithUnsignedShort:language]];
    [query whereKey:@"Name"     notEqualTo:             @""];
    [query whereKey:@"UUID"     notEqualTo:             [NSString stringWithUTF8String:obj_id.c_str()]];
    [query orderByDescending:@"updatedAt"];
    query.limit = 1000;
    
    if (fromDatastore) [query fromLocalDatastore];
    
    [query findObjectsInBackgroundWithBlock:^(NSArray *results, NSError *error) {
        NSString* map_str = [NSString stringWithFormat:@"Map%@", nsmin.stringValue];
        if (!fromDatastore && !error) {
            [PFObject unpinAllObjectsInBackgroundWithName:map_str block:^(BOOL succeeded, NSError* error) {
                [PFObject pinAllInBackground:results withName:map_str block:^(BOOL succeeded, NSError* error) {
                    queryCallback(results, error, "Profile", query_id);
                }];
            }];
        } else {
            queryCallback(results, error, "Profile", query_id);
        }
    }];
    return query_id;
}

void IOSParseWrapperObject::setValue ( const std::string& key, const std::string& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        pfobj[[NSString stringWithUTF8String:key.c_str()]] = [NSString stringWithUTF8String:value.c_str()];
    }
}

void IOSParseWrapperObject::setValue   ( const std::string& key, const unsigned int& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        pfobj[[NSString stringWithUTF8String:key.c_str()]] = [NSNumber numberWithUnsignedInt:value];
    }
}
void IOSParseWrapperObject::setValue   ( const std::string& key, const unsigned short& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        pfobj[[NSString stringWithUTF8String:key.c_str()]] = [NSNumber numberWithUnsignedShort:value];
    }
}
void IOSParseWrapperObject::setValue   ( const std::string& key, const long& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        pfobj[[NSString stringWithUTF8String:key.c_str()]] = [NSNumber numberWithLong:value];
    }
}

void IOSParseWrapperObject::setValue   ( const std::string& key, const Date& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        pfobj[[NSString stringWithUTF8String:key.c_str()]] = [NSDate dateWithTimeIntervalSince1970:value.getTime()];
    }
}

void IOSParseWrapperObject::setValue   ( const std::string& key, const std::set<std::string>& value )
{
    NSMutableArray* val_array = [NSMutableArray array];
    for ( auto& i : value ) {
        [val_array addObject:[NSString stringWithUTF8String:i.c_str()]];
    }
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        pfobj[[NSString stringWithUTF8String:key.c_str()]] = val_array;
    }
}

void IOSParseWrapperObject::getValue  ( const std::string& key, std::set<std::string>& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        NSArray* str = pfobj[[NSString stringWithUTF8String:key.c_str()]];
        if (str) {
            for ( NSString* val in str ) {
                value.insert(val.UTF8String);
            }
        }
    }
}

void IOSParseWrapperObject::getValue  ( const std::string& key, std::string& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        NSString* str = pfobj[[NSString stringWithUTF8String:key.c_str()]];
        if (str) value = str.UTF8String;
    }
}

void IOSParseWrapperObject::getValue   ( const std::string& key, unsigned int& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        NSNumber* str = pfobj[[NSString stringWithUTF8String:key.c_str()]];
        if (str) value = str.unsignedIntValue;
    }
}
void IOSParseWrapperObject::getValue   ( const std::string& key, unsigned short& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        NSNumber* str = pfobj[[NSString stringWithUTF8String:key.c_str()]];
        if (str) value = str.unsignedShortValue;
    }
}
void IOSParseWrapperObject::getValue   ( const std::string& key, long& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        NSNumber* str = pfobj[[NSString stringWithUTF8String:key.c_str()]];
        if (str) value = str.longValue;
    }
}
void IOSParseWrapperObject::getValue   ( const std::string& key, Date& value )
{
    PFObject* pfobj = (PFObject*)_pobj;
    if (pfobj) {
        NSDate* str = pfobj[[NSString stringWithUTF8String:key.c_str()]];
        if (str) value.setTime([str timeIntervalSince1970]);
    }
}

std::string     IOSParseWrapperObject::getClassName        ( )
{
    PFObject* pfobj = (PFObject*)_pobj;
    return [[pfobj parseClassName] UTF8String];
}
std::string     IOSParseWrapperObject::getObjectId         ( )
{
    PFObject* pfobj = (PFObject*)_pobj;
    return [[pfobj objectId] UTF8String];
}
time_t          IOSParseWrapperObject::getUpdatedAt        ( )
{
    PFObject* pfobj = (PFObject*)_pobj;
    return [[pfobj updatedAt] timeIntervalSince1970];
}
time_t          IOSParseWrapperObject::getCreatedAt        ( )
{
    PFObject* pfobj = (PFObject*)_pobj;
    return [[pfobj createdAt] timeIntervalSince1970];
}

IOSParseWrapperObject::IOSParseWrapperObject( void* pobj ) :
ParseWrapperObject( ),
_pobj(nullptr)
{
    PFObject* p_obj = (PFObject*) pobj;
    [p_obj retain];
    _pobj = p_obj;
}
IOSParseWrapperObject::~IOSParseWrapperObject()
{
    PFObject* p_obj = (PFObject*) _pobj;
    [p_obj release];
}

void IOSParseWrapperObject::setPObject          ( void* pobj )
{
    if (_pobj) {
        PFObject* p_obj = (PFObject*) _pobj;
        [p_obj release];
    }
    PFObject* p_obj = (PFObject*) pobj;
    _pobj = p_obj;
    [p_obj retain];
}

