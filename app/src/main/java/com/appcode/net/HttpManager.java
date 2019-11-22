package com.appcode.net;

import android.util.Log;

import java.net.URL;

public class HttpManager {

	public static void openUrl(String httpUrl){
		try{
			URL url = new URL(httpUrl);

		}catch (Exception e){
			Log.e("HttpManager",e.getMessage());
		}

	}

}
