package com.appcode;

import android.app.Application;

import com.appcode.utils.PermissionManager;

public class AppCodeApplication extends Application {
	@Override
	public void onCreate() {
		super.onCreate();
		PermissionManager.init(this);
	}
}
