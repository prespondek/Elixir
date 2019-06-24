package com.bridge.platform.util;

import java.io.FileOutputStream;
import java.util.*;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxHelper;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.AsyncTask;
import android.util.Log;


public class PlatformUtilBridge
{
	private static final String TAG = "com.bridge.platform.util.PlatformUtilBridge";

	public static void downloadUrl(final String url, final String path) 
	{
		Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            public void run() {
				DownloadTask downloadTask = new DownloadTask(url, path);
		        downloadTask.execute();
            }
		});
	}
	
	public static String getVersionNumber () 
	{
		PackageInfo pInfo = null;
		try {
			pInfo = Cocos2dxActivity.getContext().getPackageManager().getPackageInfo(Cocos2dxActivity.getContext().getPackageName(), 0);
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}
		return pInfo.versionName;
	}
	
	public static String getBuildNumber () 
	{
		PackageInfo pInfo = null;
		try {
			pInfo = Cocos2dxActivity.getContext().getPackageManager().getPackageInfo(Cocos2dxActivity.getContext().getPackageName(), 0);
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}
		return String.valueOf(pInfo.versionCode);
	}
	
	public static native void downloadComplete (String url, String path, boolean success);
	
	public static String createUUID ( )
	{
		return UUID.randomUUID().toString();
	}
}

class DownloadTask extends AsyncTask<String, Integer, String> {

    private String path;
    private String url;
	private static final String TAG = "com.bridge.platform.util.DownloadTask";


    public DownloadTask(String url, String path) {
    	this.url = url;
        this.path = path;
    }
    
    protected void onPostExecute (final String result)
    {
        Log.d(TAG, "downloadComplete: path:" + path + " url:" + url);
        Cocos2dxHelper.runOnGLThread(new Runnable() {
			 public void run() {
				 if (result == null)
				 PlatformUtilBridge.downloadComplete(url, path, true);
			 }
        });
    }
    protected void onCancelled (String result)
    {
        Log.w(TAG, "downloadCancelled");
        Cocos2dxHelper.runOnGLThread(new Runnable() {
			 public void run() {
				 PlatformUtilBridge.downloadComplete(url, path, false);
			 }
        });
    }

    @Override
    protected String doInBackground(String... sUrl) {
        InputStream input = null;
        OutputStream output = null;
        HttpURLConnection connection = null;
        try {
            URL url = new URL(this.url);
            connection = (HttpURLConnection) url.openConnection();
            connection.connect();

            // expect HTTP 200 OK, so we don't mistakenly save error report
            // instead of the file
            if (connection.getResponseCode() != HttpURLConnection.HTTP_OK) {
                return "Server returned HTTP " + connection.getResponseCode()
                        + " " + connection.getResponseMessage();
            }

            // this will be useful to display download percentage
            // might be -1: server did not report the length
            int fileLength = connection.getContentLength();

            // download the file
            input = connection.getInputStream();
            output = new FileOutputStream(this.path);

            byte data[] = new byte[4096];
            long total = 0;
            int count;
            while ((count = input.read(data)) != -1) {
                // allow canceling with back button
                if (isCancelled()) {
                    input.close();
                    return null;
                }
                total += count;
                // publishing the progress....
                if (fileLength > 0) // only if total length is known
                    publishProgress((int) (total * 100 / fileLength));
                output.write(data, 0, count);
            }
        } catch (Exception e) {
            return e.toString();
        } finally {
            try {
                if (output != null)
                    output.close();
                if (input != null)
                    input.close();
            } catch (IOException ignored) {
            }

            if (connection != null)
                connection.disconnect();
        }
        return null;
    }
}



