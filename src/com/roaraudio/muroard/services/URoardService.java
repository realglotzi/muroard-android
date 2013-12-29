package com.roaraudio.muroard.services;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.IBinder;
import android.util.Log;

public class URoardService extends Service {

	private static final String TAG = "URoardService";

    int mStartMode;       // indicates how to behave if the service is killed
    IBinder mBinder;      // interface for clients that bind
    boolean mAllowRebind; // indicates whether onRebind should be used
    URoardAsyncTask asyncTask;
    
	@Override
	public void onCreate() {
		asyncTask = new URoardAsyncTask();
	}

    @Override
    // The service is starting, due to a call to startService()
    public int onStartCommand(Intent intent, int flags, int startId) {
    	
    	Log.d(TAG, "onStartCommand =>");
    	
    	asyncTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        return mStartMode;
    }

    @Override
    public IBinder onBind(Intent intent) {
        // A client is binding to the service with bindService()
        return mBinder;
    }
    
    @Override
    public boolean onUnbind(Intent intent) {
        // All clients have unbound with unbindService()
        return mAllowRebind;
    }
    
    @Override
    public void onRebind(Intent intent) {
        // A client is binding to the service with bindService(),
        // after onUnbind() has already been called
    }
    
    @Override
    public void onDestroy() {
        // The service is no longer used and is being destroyed
    }
}
