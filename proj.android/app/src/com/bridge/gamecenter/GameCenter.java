package com.bridge.gamecenter;

import android.app.AlertDialog;
import android.content.Context;
import android.util.Log;
import android.content.Intent;

import org.cocos2dx.lib.Cocos2dxActivity;

import com.google.android.gms.auth.api.Auth;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.auth.api.signin.GoogleSignInResult;
import com.google.android.gms.games.Games;
import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;
import android.support.annotation.NonNull;


public class GameCenter {

    private static GameCenter instance = new GameCenter();
    private static final String TAG = "com.bridge.gamecenter";
    private boolean isSignedIn = false;
    private final static Cocos2dxActivity activity = (Cocos2dxActivity) Cocos2dxActivity.getContext();
    private static GoogleSignInAccount mSignedInAccount = null;
    private final static int REQUEST_ACHIEVEMENTS = 9003;
    private final static int REQUEST_LEADERBOARD = 9004;
    private GoogleSignInClient mGoogleSignInClient = mGoogleSignInClient = GoogleSignIn.getClient(Cocos2dxActivity.getContext(),
            new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN).build());

    // request codes we use when invoking an external activity
    private static final int RC_UNUSED = 5001;
    private static final int RC_SIGN_IN = 9001;

    /**
     *
     * Interface used for C++
     *
     */
    private GameCenter() {
    }

    private void onConnected(GoogleSignInAccount googleSignInAccount) {
        Log.d(TAG, "onConnected(): connected to Google APIs");
        if (mSignedInAccount != googleSignInAccount) {
            mSignedInAccount = googleSignInAccount;
        }
        activity.runOnGLThread(new Runnable() {
            public void run() {
                loginSucceeded(true);
            }
        });
    }

    private void onDisconnected() {

        Log.d(TAG, "onDisconnected()");

        mSignedInAccount = null;
        activity.runOnGLThread(new Runnable() {
            public void run() {
                loginSucceeded(false);
            }
        });
    }

    public static void login()
    {
    	instance.signIn();
    }

    public static void signInSilently()
    {
        Log.d(TAG, "signInSilently()");
        //getGameHelper().beginUserInitiatedSignIn();

        instance.activity.runOnUiThread(new Runnable() {
            public void run() {

                instance.mGoogleSignInClient.silentSignIn().addOnCompleteListener(instance.activity,
                new OnCompleteListener<GoogleSignInAccount>() {
                    @Override
                    public void onComplete(@NonNull Task<GoogleSignInAccount> task) {
                        if (task.isSuccessful()) {
                            Log.d(TAG, "signInSilently(): success");
                            instance.onConnected(task.getResult());
                        } else {
                            Log.d(TAG, "signInSilently(): failure " + task.getException().getLocalizedMessage());
                            instance.onDisconnected();
                        }
                    }
                });
            }
        });
    }

    private static void signOut() {
        Log.d(TAG, "signOut()");

        if (!instance.isSignedIn()) {
            Log.w(TAG, "signOut() called, but was not signed in!");
            return;
        }
        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                instance.mGoogleSignInClient.signOut().addOnCompleteListener(instance.activity,
                new OnCompleteListener<Void>() {
                    @Override
                    public void onComplete(@NonNull Task<Void> task) {
                        boolean successful = task.isSuccessful();
                        Log.d(TAG, "signOut(): " + (successful ? "success" : "failed"));

                        instance.onDisconnected();
                    }
                });
            }
        });
    }

    private static boolean isSignedIn() {
        boolean login = GoogleSignIn.getLastSignedInAccount(instance.activity) != null;
        return login;
    }


    public static boolean showAchievements() {
        if (!instance.isSignedIn()) {
            return false;
        }
        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                Games.getAchievementsClient(instance.activity,
                GoogleSignIn.getLastSignedInAccount(instance.activity))
                .getAchievementsIntent()
                .addOnSuccessListener(new OnSuccessListener<Intent>() {
                    @Override
                    public void onSuccess(Intent intent) {
                        instance.activity.startActivityForResult(intent, REQUEST_ACHIEVEMENTS);
                    }
                });
            }
        });
        return true;
    }

    public static void postAchievement(final String idName, final int percentComplete)
    {
        if (!instance.isSignedIn()) {
            return;
        }
        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                if (percentComplete != 100) {
                    Games.getAchievementsClient(instance.activity,
                    GoogleSignIn.getLastSignedInAccount(instance.activity))
                    .increment(idName, 1);
                } else {
                    Games.getAchievementsClient(instance.activity,
                    GoogleSignIn.getLastSignedInAccount(instance.activity))
                    .unlock(idName);
                }
            }
        });
    }

    public static String getPlayerId()
    {
        GoogleSignInAccount gc = GoogleSignIn.getLastSignedInAccount(instance.activity);
        if (gc == null) return "";
        return gc.getId();
    }

    public static void clearAllAchievements()
    {
        Log.v(TAG, "clearAllAchievements is not available on this platform");
    }

    public static boolean showScores()
    {
        if (!instance.isSignedIn()) {
            return false;
        }
        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                Games.getLeaderboardsClient(instance.activity,
                GoogleSignIn.getLastSignedInAccount(instance.activity))
                .getAllLeaderboardsIntent()
                .addOnSuccessListener(new OnSuccessListener<Intent>() {
                    @Override
                    public void onSuccess(Intent intent) {
                        instance.activity.startActivityForResult(intent, REQUEST_LEADERBOARD);
                    }
                });
            }
        });
        return true;
    }

    public static void postScore(final String idName, final int score)
    {
        if (!instance.isSignedIn()) {
            return;
        }

        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                Games.getLeaderboardsClient(instance.activity,
                        GoogleSignIn.getLastSignedInAccount(instance.activity))
                        .submitScore(idName, 1337);
            }
        });
    }

    public static void showGameCenter ()
    {
        if (!instance.isSignedIn()) {
            return;
        }
        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                Games.getGamesClient(instance.activity, GoogleSignIn.getLastSignedInAccount(instance.activity))
                .getSettingsIntent()
                .addOnFailureListener(new OnFailureListener() {
                    @Override
                    public void onFailure(@NonNull Exception e) {
                        instance.onDisconnected();
                    }
                });
            }
        });
    }

    private void signIn() {
        if (instance.isSignedIn()) {
            loginSucceeded(true);
            return;
        }
        instance.activity.runOnUiThread(new Runnable() {
            public void run() {
                GoogleSignInClient signInClient = GoogleSignIn.getClient(instance.activity,
                        GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN);
                Intent intent = signInClient.getSignInIntent();
                instance.activity.startActivityForResult(intent, RC_SIGN_IN);
            }
        });
    }

    public static void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == RC_SIGN_IN) {
            GoogleSignInResult result = Auth.GoogleSignInApi.getSignInResultFromIntent(data);
            if (result.isSuccess()) {
                instance.onConnected(instance.mSignedInAccount);
            } else {

                instance.onDisconnected();
            }
        }
    }

    public static void clearAllScores()
    {
        Log.v(TAG, "clearAllScores is not available on this platform");
    }

    /**
     *
     * Public interface to integrate this into the main activity
     *
     */

    public static void onResume() {
        instance.signInSilently();
    }

    
    public static native void loginSucceeded(boolean login);



}