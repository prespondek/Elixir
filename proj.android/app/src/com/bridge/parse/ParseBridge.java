package com.bridge.parse;

import java.sql.Date;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import com.parse.FindCallback;
import com.parse.GetCallback;
import com.parse.facebook.ParseFacebookUtils;
import com.parse.ParseObject;
import com.parse.ParseQuery;
import com.parse.ParseUser;
import com.parse.ParseAnonymousUtils;
import com.parse.LogInCallback;
import com.parse.ParseException;
import com.parse.SaveCallback;
import com.bridge.facebook.FacebookBridge;
import com.facebook.AccessToken;

import android.util.Log;

public class ParseBridge {
    private static final String TAG = "com.bridge.parse";

    public static boolean isLoggedIn() {
        ParseUser currentUser = ParseUser.getCurrentUser();
        boolean loggedin = ParseFacebookUtils.isLinked(currentUser);
        return loggedin;
    }

    public static void logOut() {
        ParseUser.logOut();
        FacebookBridge.logOut();
        AccessToken.setCurrentAccessToken(null);
    }

    public static LogInCallback logInCallback() {
        return new LogInCallback() {
            @Override
            public void done(final ParseUser user, final ParseException err) {
                Cocos2dxHelper.runOnGLThread(new Runnable() {
                    public void run() {
                        if (user == null && err == null) {
                            Log.d(TAG, "Uh oh. The user cancelled the Facebook login.");
                            parseLoginCallback(1, false);
                        } else if (user == null && err != null) {
                            Log.d(TAG, "Error.");
                            parseLoginCallback(2, false);
                        } else {
                            if (user.isNew()) {
                                Log.e(TAG, "parseLoginCallback");
                                parseLoginCallback(0, true);
                            } else {
                                Log.d(TAG, "User logged in through Facebook!");
                                parseLoginCallback(0, false);
                            }
                        }
                    }
                });
            }
        };
    }

    public static void logInWithFacebook() {
        logOut();
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
                List<String> premission = Arrays.asList("public_profile", "email", "user_friends");
                ParseFacebookUtils.logInWithReadPermissionsInBackground(
                        (Cocos2dxActivity) Cocos2dxActivity.getContext(),
                        premission,
                        logInCallback());
            }
        });
    }

    public static void logIn() {
        Log.d(TAG, "ParseBridge::logIn()");
        ParseUser currentUser = ParseUser.getCurrentUser();
        if (currentUser != null) {
            parseLoginCallback(0, true);
        } else {
            Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
                public void run() {
                    ParseAnonymousUtils.logIn(logInCallback());
                }
            });
        }
    }

    public static void save(final String[] key_array, final String[] value_array) {
        Log.d(TAG, "ParseBridge::save()");
        ParseUser currentUser = ParseUser.getCurrentUser();
        for (int i = 0; i < key_array.length; i++) {
            currentUser.put(key_array[i], value_array[i]);
        }

        currentUser.pinInBackground();
        currentUser.saveEventually();
    }

    public static void unpinAllObjectsWithName(String name) {
        try {
            ParseObject.unpinAll(name);
        } catch (ParseException e) {
            e.printStackTrace();
        }
    }

    public static ParseObject createObject(String class_name, String object_id) {
        if (object_id != null && object_id.length() > 0)
            return ParseObject.createWithoutData(class_name, object_id);
        else
            return ParseObject.create(class_name);
    }

    public static ParseObject getUserObject() {
        ParseUser user = ParseUser.getCurrentUser();
        return user;
    }

    public static void saveObject(final int query_id, final ParseObject obj, boolean datastore) {
        if (datastore) {
            try {
                obj.pin();
            } catch (ParseException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        } else {
            Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
                public void run() {
                    obj.saveInBackground(new SaveCallback() {
                        public void done(final ParseException e) {
                            Cocos2dxHelper.runOnGLThread(new Runnable() {
                                public void run() {
                                    objectSaved(query_id, obj.getClassName(), obj.getObjectId(), e == null);
                                }
                            });
                        }
                    });
                }
            });
        }
    }


    public static void saveObjectEventually(ParseObject obj) {
        obj.saveEventually();
    }

    public static void deleteWithObjectId(String class_name, String object_id) {
        ParseObject pobj = ParseObject.createWithoutData(class_name, object_id);
        pobj.deleteEventually();
    }

    public static void loadWithObjectId(final int query_id, final String class_name, final String object_id) {
        ParseObject pobj = ParseObject.createWithoutData(class_name, object_id);
        fetchObjectFromLocalDatastore(pobj);
    }

    public static void loadUser() {
        ParseObject pobj = ParseUser.getCurrentUser();
        fetchObjectFromLocalDatastore(pobj);
    }

    public static void fetchObjectFromLocalDatastore(final ParseObject pobj) {
        try {
            pobj.fetchFromLocalDatastore();
        } catch (ParseException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public static void fetchObjectInBackground(final int query_id, final ParseObject pobj) {
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
                pobj.fetchInBackground(new GetCallback<ParseObject>() {
                    public void done(final ParseObject obj, final ParseException e) {
                        Cocos2dxHelper.runOnGLThread(new Runnable() {
                            public void run() {
                                queryStarted(query_id, obj.getClassName(), e == null);
                                queryLoaded(query_id, obj);
                                queryComplete(query_id, obj.getClassName(), e == null);
                            }
                        });
                    }
                });
            }
        });
    }

    public static void pinObject(ParseObject obj) {
        try {
            obj.pin();
        } catch (ParseException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public static void findObjectWithKey(final int query_id, final String table, final String key, final String value, final boolean fromDataStore) {
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
                ParseQuery<ParseObject> query = ParseQuery.getQuery(table);
                if (fromDataStore) query.fromLocalDatastore();
                query.whereEqualTo(key, value);
                query.findInBackground(new FindCallback<ParseObject>() {
                    @Override
                    public void done(final List<ParseObject> arg0, final ParseException arg1) {
                        Cocos2dxHelper.runOnGLThread(new Runnable() {
                            public void run() {
                                // TODO Auto-generated method stub
                                queryStarted(query_id, table, arg1 == null);
                                for (ParseObject obj : arg0) {
                                    queryLoaded(query_id, obj);
                                }
                                queryComplete(query_id, table, arg1 == null);
                            }
                        });
                    }
                });
            }
        });

    }

    public static void getMapScores(final int query_id, final String obj_id, final int language, final int min, final int max, final boolean fromDatastore) {
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
                ParseQuery<ParseObject> query = ParseQuery.getQuery("Profile");
                query.whereLessThanOrEqualTo("MaxLevel", max);
                query.whereGreaterThanOrEqualTo("Level", min);
                query.whereEqualTo("Language", language);
                query.whereNotEqualTo("Name", "");
                query.whereNotEqualTo("UUID", obj_id);
                query.orderByDescending("updatedAt");
                query.setLimit(1000);

                if (fromDatastore) query.fromLocalDatastore();
                query.findInBackground(new FindCallback<ParseObject>() {
                    @Override
                    public void done(final List<ParseObject> arg0, final ParseException arg1) {
                        // TODO Auto-generated method stub
                        Cocos2dxHelper.runOnGLThread(new Runnable() {
                            public void run() {
                                queryStarted(query_id, "Profile", arg1 == null);
                                for (ParseObject obj : arg0) {
                                    queryLoaded(query_id, obj);
                                }
                                queryComplete(query_id, "Profile", arg1 == null);
                            }
                        });
                    }
                });
            }
        });
    }

    public static void getFriends(final int query_id, final String[] friend_indices, final boolean fromDatastore) {
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
                List<String> friend_list = Arrays.asList(friend_indices);
                ParseQuery<ParseObject> parse_query = ParseQuery.getQuery("Profile");
                ParseQuery<ParseObject> fb_query = ParseQuery.getQuery("Profile");
                parse_query.whereContainedIn("UUID", friend_list);
                fb_query.whereContainedIn("FacebookId", friend_list);
                List<ParseQuery<ParseObject>> queries = new ArrayList<ParseQuery<ParseObject>>();
                queries.add(parse_query);
                queries.add(fb_query);
                ParseQuery<ParseObject> query = ParseQuery.or(queries);
                query.orderByAscending("updatedAt");

                query.findInBackground(new FindCallback<ParseObject>() {
                    @Override
                    public void done(final List<ParseObject> arg0, final ParseException e) {
                        // TODO Auto-generated method stub
                        Cocos2dxHelper.runOnGLThread(new Runnable() {
                            public void run() {
                                if (!fromDatastore && e == null) {
                                    try {
                                        ParseObject.unpinAll("Friends");
                                        ParseObject.pinAll("Friends", arg0);
                                    } catch (ParseException e1) {
                                        // TODO Auto-generated catch block
                                        e1.printStackTrace();
                                    }
                                }
                                queryStarted(query_id, "Profile", e == null);

                                if (arg0 != null) {
                                    for (ParseObject obj : arg0) {
                                        queryLoaded(query_id, obj);
                                    }
                                }
                                queryComplete(query_id, "Profile", e == null);
                            }
                        });
                    }
                });
            }
        });

    }

    static boolean hasValue(ParseObject obj, String name) {
        return obj.has(name);
    }

    static String getClassName(ParseObject obj) {
        return obj.getClassName();
    }

    static String getObjectId(ParseObject obj) {
        return obj.getObjectId();
    }

    static long getUpdatedAt(ParseObject obj) {
        return obj.getUpdatedAt().getTime();
    }

    static long getCreatedAt(ParseObject obj) {
        return obj.getCreatedAt().getTime();
    }

    static Object[] loadArray(ParseObject obj, String name) {
        List<String> str = obj.getList(name);
        if (str == null) return new String[1];
        else return str.toArray();
    }

    static String loadString(ParseObject obj, String name) {
        String str = obj.getString(name);
        if (str == null) return "";
        else return str;
    }

    static int loadInt(ParseObject obj, String name) {
        return obj.getInt(name);
    }

    static short loadShort(ParseObject obj, String name) {
        return (short) obj.getInt(name);
    }

    static long loadLong(ParseObject obj, String name) {
        return obj.getLong(name);
    }

    static long loadDate(ParseObject obj, String name) {
        java.util.Date date = obj.getDate(name);
        return date.getTime();
    }

    static void saveArray(ParseObject obj, String name, String[] value) {
        for (String str : value) {
            obj.addUnique(name, str);
        }
    }

    static void saveString(ParseObject obj, String name, String value) {
        obj.put(name, value);
    }

    static void saveInt(ParseObject obj, String name, int value) {
        obj.put(name, value);
    }

    static void saveShort(ParseObject obj, String name, short value) {
        obj.put(name, value);
    }

    static void saveLong(ParseObject obj, String name, long value) {
        obj.put(name, value);
    }

    static void saveDate(ParseObject obj, String name, long value) {
        java.util.Date date = new java.util.Date(value);
        obj.put(name, date);
    }

    public static native void parseLoginCallback(int success, boolean isnew);

    public static native void objectLoaded(int query_id, ParseObject obj, String name, String object_id, boolean success);

    public static native void objectSaved(int query_id, String class_name, String object_id, boolean success);

    public static native void queryLoaded(int query_id, ParseObject obj);

    public static native void queryComplete(int query_id, String table, boolean success);

    public static native void queryStarted(int query_id, String table, boolean success);

}