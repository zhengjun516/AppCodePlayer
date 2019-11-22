package com.appcode.utils;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Process;

public class PermissionManager {
	private static Context sContext;

	public static void init(Context context){
		sContext = context;
	}

	public static boolean checkPermissionStr(String permissionStr) {
		if(sContext == null){
			throw  new RuntimeException("PermissionManager not init");
		}
		try {
			if (Build.VERSION.SDK_INT < 23) {
				return true;
			}
			if (permissionStr != null) {
				int result = sContext.checkPermission(permissionStr, Process.myPid(), Process.myUid());
				return result == PackageManager.PERMISSION_GRANTED;
			}
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return true;
		}
	}

	public static boolean isExternalStorageGranted() {
		return checkPermissionStr("android.permission.WRITE_EXTERNAL_STORAGE") && checkPermissionStr("android.permission.READ_EXTERNAL_STORAGE");
	}

	public static boolean isContactsGranted() {
		return checkPermissionStr("android.permission.READ_CONTACTS");
	}
}
