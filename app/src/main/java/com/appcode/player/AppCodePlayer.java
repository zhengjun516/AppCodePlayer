package com.appcode.player;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class AppCodePlayer implements SurfaceHolder.Callback{

	static {
		System.loadLibrary("AppCodePlayer");
	}

	private String dataSource;
	private SurfaceView mSurfaceView;
	private SurfaceHolder mSurfaceHolder;
	private OnPreparedListener preparedListener;


	public void setSurfaceView(SurfaceView surfaceView){
		if(this.mSurfaceHolder != null){
			mSurfaceHolder.removeCallback(this);
		}
		mSurfaceView = surfaceView;
		mSurfaceHolder = surfaceView.getHolder();
		native_setSurface(mSurfaceHolder.getSurface());
		mSurfaceHolder.addCallback(this);
	}

	public void setDataSource(String dataSource){
		this.dataSource = dataSource;
	}

	public void prepare(){
		Log.d("AppCodePlayer","dataSource:"+dataSource);
		native_prepare(dataSource);
	}

	public void start(){
		native_start();
	}

	public void pause(){

	}

	public void stop(){

	}

	public void release(){
		mSurfaceHolder.removeCallback(this);
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {

	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		native_setSurface(holder.getSurface());
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {

	}

	public void onPrepared(){
		if(preparedListener != null){
			preparedListener.onPrepared();
		}
	}

	public void onError(int errorCode){

	}

	public native void native_prepare(String dataSource);
	public native void native_start();
	public native void native_setSurface(Surface surface);

	public native String getUrlProtocolInfo();
	public native String getAVFormatInfo();
	public native String getAVCodecInfo();


	public void setOnPreparedListener(OnPreparedListener onPreparedListener){
		this.preparedListener = onPreparedListener;
	}

	public interface OnPreparedListener{
		void onPrepared();
	}

}
