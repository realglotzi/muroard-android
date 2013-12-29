package com.roaraudio.muroard.services;

import com.roaraudio.muroard.NativeUtils;

import android.os.AsyncTask;
import android.util.Log;

public class URoardAsyncTask extends AsyncTask<Void, Void, Integer> {

	private static final String TAG = "URoardAsyncTask";
	
	private NativeUtils nativeUtils = new NativeUtils();
	
	@Override
	protected Integer doInBackground(Void... arg0) {
		Log.d(TAG, "doInBackground start");

		nativeUtils.startMuroarD("192.168.1.28", 3000);
		
		Integer retCode = null;
		Log.d(TAG, "doInBackground end " + retCode );
		
		return retCode;
	}

}
