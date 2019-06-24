package com.bridge.facebook;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;
import org.cocos2dx.cpp.*;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONArray;

import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import com.facebook.*;
import com.facebook.login.*;
import com.facebook.share.model.ShareLinkContent;
import com.facebook.share.model.AppInviteContent;
import com.facebook.share.model.GameRequestContent;
import com.facebook.share.widget.AppInviteDialog;
import com.facebook.share.widget.GameRequestDialog;
import com.facebook.share.widget.ShareDialog;

public class FacebookBridge 
{
	private static final String TAG = "com.bridge.facebook";
	public static AccessTokenTracker accessTokenTracker;
    public static native void userLoggedIn 			(boolean success);
    public static native void recieveGameRequests 	(Object[] reqs, boolean success);
    public static native void recieveFriendData 	(Object[] reqs, boolean success);
    public static native void recieveUserData 		(Object [] data, boolean success);
    public static native boolean incomingUrl 		(String url);
    public static native void gameRequestRemoved 	(String req, boolean success);
    public static native void gameRequestRemoveComplete ();  
    public static native void errorCallback 		(int code);
    public static native void gameRequestComplete   (boolean complete);


	public static boolean isLoggedIn ( )
    {
	    Log.d(TAG, "isLoggedIn()");
        AccessToken accessToken = AccessToken.getCurrentAccessToken();
    	if (accessToken == null) return false;
    	return true;
    }
    
    public static void incomingUrl (final Uri targetUrl, final Bundle links, final Bundle extras)
    {
	    Log.d(TAG, "incomingUrl()");
    	if ( targetUrl != null ) {
    		Cocos2dxHelper.runOnGLThread(new Runnable() {
                public void run() { incomingUrl(targetUrl.toString()); }
    		});
    	}
    }

	public static void logIn ( )
	{
	    Log.d(TAG, "logIn()");
		 LoginManager.getInstance().logInWithReadPermissions((Cocos2dxActivity)Cocos2dxActivity.getContext(), Arrays.asList("public_profile", "user_friends"));
	}
	
	public static void logOut( )
	{
	    Log.d(TAG, "logOut()");
		LoginManager.getInstance().logOut();
	}

	public static void postLink (final String link)
	{
		Log.d(TAG, "sendGameRequest()");
		Cocos2dxHelper.runOnGLThread(new Runnable() {
			public void run() {
				ShareLinkContent content = new ShareLinkContent.Builder()
						.setContentUrl(Uri.parse(link))
						.build();
				ShareDialog.show((Cocos2dxActivity)Cocos2dxActivity.getContext(), content);
			}
		});
	}

	public static void sendGameRequest ( final int request_type,final String message, final String title, final String recipent)
	{
	    Log.d(TAG, "sendGameRequest()");
	    Cocos2dxHelper.runOnGLThread(new Runnable() {
            public void run() {
				GameRequestContent content = new GameRequestContent.Builder()
			    .setMessage(message)
			    .setTitle(title)
			    .setRecipients(Collections.singletonList(recipent))
				.setData(Integer.toString(request_type))
			    .build();
				((AppActivity)Cocos2dxActivity.getContext()).requestDialog.show(content);
            }
        });
	}
	

	public static void refreshAccessToken()
	{
	    Log.d(TAG, "refreshAccessToken()");
	    accessTokenTracker = new AccessTokenTracker() {
	        @Override
	        protected void onCurrentAccessTokenChanged(AccessToken oldAccessToken, AccessToken currentAccessToken) {
	        	accessTokenTracker.stopTracking();
	        	if(currentAccessToken == null) {
	        		errorCallback(8);
	        		userLoggedIn (false);
	            }
	            else {
	            	userLoggedIn (true);
	            }
	        }
	    };

        AccessToken.refreshCurrentAccessTokenAsync();
	}
	
	public static void openAppInviteDialog( final String app_link, final String img_url )
	{
	    Log.d(TAG, "openAppInviteDialog()");
        Cocos2dxHelper.runOnGLThread(new Runnable() {
            public void run() {
				if (AppInviteDialog.canShow()) {
				    AppInviteContent content = new AppInviteContent.Builder()
				                .setApplinkUrl(app_link)
				                .setPreviewImageUrl(img_url)
				                .build();
				    AppInviteDialog.show((Cocos2dxActivity)Cocos2dxActivity.getContext(), content);
				}
            }
        });
	}
	
	public static void getFriendData( )
	{
	    Log.d(TAG, "getFriendData()");
	    Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
				AccessToken accessToken = AccessToken.getCurrentAccessToken();
				GraphRequest request = GraphRequest.newMyFriendsRequest(
				        accessToken,
				        new GraphRequest.GraphJSONArrayCallback() {
				            @Override
				            public void onCompleted(final JSONArray object, final GraphResponse response) {
				            	Cocos2dxHelper.runOnGLThread(new Runnable() {
				                    public void run() {
										ArrayList<String[]> friends_data = new ArrayList<String[]>();
										if (response.getError() != null) {
										    Log.d(TAG, "recieveFriendData()");
											recieveFriendData(friends_data.toArray(), false);
											return;
										}
										try {
											for ( int i = 0; i < object.length(); i++) {
												String [] friend_data = new String [3];
												JSONObject friend = object.getJSONObject(i);
												friend_data[0] = friend.getString("name");
												friend_data[1] = friend.getString("id");
												JSONObject friend_picture = friend.getJSONObject("picture").getJSONObject("data");
												if (friend_picture.getBoolean("is_silhouette") == false) {
													friend_data[2] = friend_picture.getString("url");
												}
												friends_data.add(friend_data);
											}
										    Log.d(TAG, "recieveFriendData()");
											recieveFriendData(friends_data.toArray(), true);
										}
										catch (JSONException e) {
											// TODO Auto-generated catch block
											e.printStackTrace();
										}
				                    }
				            	});
				            }
				        });
				Bundle parameters = new Bundle();
				parameters.putString("fields", "name,id,picture.width(200).height(200)");
				request.setParameters(parameters);
				request.executeAsync();
            }
	    });
	}

	public static void getUserData( )
	{
	    Log.d(TAG, "getUserData()");
        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
				AccessToken accessToken = AccessToken.getCurrentAccessToken();
				GraphRequest request = GraphRequest.newMeRequest(
				        accessToken,
				        new GraphRequest.GraphJSONObjectCallback() {
				            @Override
				            public void onCompleted(final JSONObject object, final GraphResponse response) {
				            	Cocos2dxHelper.runOnGLThread(new Runnable() {
				                    public void run() {
										String[] user_data = new String[4];
										if (response.getError() != null) {
										    Log.d(TAG, "recieveUserData()");
											recieveUserData(user_data, false);
											return;
										}
										try {
											user_data[0] = object.getString("name");
											user_data[1] = object.getString("id");
											JSONObject picture = object.getJSONObject("picture").getJSONObject("data");
											if (picture.getBoolean("is_silhouette") == false) {
												user_data[2] = picture.getString("url");
											}
											JSONArray permission = object.getJSONObject("permissions").getJSONArray("data");
											for ( int i = 0; i < permission.length(); i++ ) {
												JSONObject d = (JSONObject)permission.get(i);
												if (d.getString("status").contentEquals("granted")) {
													user_data[3] += d.getString("permission");
												}
											}
											Log.d(TAG, "recieveUserData()");
											recieveUserData(user_data, true);
										}
										catch (JSONException e) {
											// TODO Auto-generated catch block
											e.printStackTrace();
										}
				                    }
				            	});
				            }
				        });
				Bundle parameters = new Bundle();
				parameters.putString("fields", "permissions,name,id,picture.width(200).height(200)");
				request.setParameters(parameters);
				request.executeAsync();
            }
        });

	}
	public static void deleteGameRequest ( final String [] req_ids )
	{
	    Log.d(TAG, "deleteGameRequest()");
	    Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
				AccessToken accessToken = AccessToken.getCurrentAccessToken();
				ArrayList<GraphRequest> reqs = new ArrayList<GraphRequest>();
				for ( int i = 0; i < req_ids.length; i++ ) {
			        final String req_id = req_ids[i];
					GraphRequest request = GraphRequest.newDeleteObjectRequest(
					        accessToken,
					        req_id,
					        new GraphRequest.Callback() {
								public void onCompleted(final GraphResponse response) {
									Cocos2dxHelper.runOnGLThread(new Runnable() {
					                    public void run() {
											boolean success = response.getError() == null;
										    Log.d(TAG, "gameRequestRemoved()");
											gameRequestRemoved(req_id, success);
					                    }
									});
								}
							}
				        );
					reqs.add(request);
				}
				GraphRequestBatch batch = new GraphRequestBatch(reqs);
				batch.addCallback(new GraphRequestBatch.Callback() {
				    @Override
				    public void onBatchCompleted(final GraphRequestBatch graphRequests) {
				    	Cocos2dxHelper.runOnGLThread(new Runnable() {
		                    public void run() {
							    Log.d(TAG, "gameRequestRemoveComplete()");
								gameRequestRemoveComplete();
		                    }
				    	});
				    }
				});
				batch.executeAsync();
            }
	    });
	}
	
	public static void getGameRequests( final String app_id ) 
	{
	    Log.d(TAG, "getGameRequests()");
	    Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
				AccessToken accessToken = AccessToken.getCurrentAccessToken();
				GraphRequest request = GraphRequest.newMeRequest(
		        accessToken,
		        new GraphRequest.GraphJSONObjectCallback() {
		            @Override
		            public void onCompleted(final JSONObject object, final GraphResponse response) {
		            	Cocos2dxHelper.runOnGLThread(new Runnable() {
		                    public void run() {
								ArrayList<String[]> reqs = new ArrayList<String[]>();
				            	if (response.getError() != null) { 
				            	    Log.d(TAG, "recieveGameRequests()");
									recieveGameRequests(reqs.toArray(),false);	
				            		return;
				            	}
				            	try {
									JSONObject app_req = object.getJSONObject("apprequests");
									JSONArray data = app_req.getJSONArray("data");
									for (int i = 0; i < data.length(); i++) {
										JSONObject req = data.getJSONObject(i);
					            		if (!app_id.equals(req.getJSONObject("application").getString("id"))) continue;
					            		String [] atom = new String [5];
					            		atom[0] = req.getString("message");
					            		atom[1] = req.getString("id");
										atom[2] = req.getString("data");
										atom[3] = req.getJSONObject("from").getString("id");
					            		atom[4] = req.getJSONObject("from").getString("name");
					            		reqs.add(atom);
					            	}
								} catch (JSONException e) {
									// TODO Auto-generated catch block
									e.printStackTrace();
								}
				            	Log.d(TAG, "recieveGameRequests()");
								recieveGameRequests(reqs.toArray(),true);	
				            }
				        });
		            }
		        });
				Bundle parameters = new Bundle();
				parameters.putString("fields", "apprequests");
				request.setParameters(parameters);
				request.executeAsync();
            }
	    });
	}
}

