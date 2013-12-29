package com.roaraudio.muroard.services;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class URoardStarter extends BroadcastReceiver {

	@Override
	public void onReceive(Context arg0, Intent arg1) {
		
		Log.d(getClass().getSimpleName(), "onReceive()");
		
		boolean startURoard    	= arg1.getBooleanExtra("startURoard", false);
		boolean stopURoard    	= arg1.getBooleanExtra("stopURoard", false);
		
		if (startURoard) {
			Log.d(getClass().getSimpleName(), "onReceive() start");
			arg0.startService(new Intent(arg0, URoardService.class));
		}
		
		if (stopURoard) {
			Log.d(getClass().getSimpleName(), "onReceive() stop");
			arg0.stopService(new Intent(arg0, URoardService.class));
		}
		
	}

}
