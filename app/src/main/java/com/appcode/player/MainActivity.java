package com.appcode.player;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.appcode.utils.PermissionManager;

public class MainActivity extends AppCompatActivity {

	// Used to load the 'native-lib' library on application startup.
	/*static {
		System.loadLibrary("native-lib");
	}*/

	private SurfaceView mSurfaceView;

	private AppCodePlayer appCodePlayer;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		initPermission();

		mSurfaceView = findViewById(R.id.mSurfaceView);

		appCodePlayer = new AppCodePlayer();
		appCodePlayer.setSurfaceView(mSurfaceView);
		appCodePlayer.setDataSource("/sdcard/Pictures/test/VID-20191108-WA0000.mp4");

		appCodePlayer.setOnPreparedListener(new AppCodePlayer.OnPreparedListener() {
			@Override
			public void onPrepared() {
				appCodePlayer.start();
				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						Toast.makeText(MainActivity.this,"可以开始播放了",Toast.LENGTH_LONG).show();;
					}
				});
			}
		});



		findViewById(R.id.mProtocolInfo).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((TextView)findViewById(R.id.mInfo)).setText(appCodePlayer.getUrlProtocolInfo());
			}
		});
		findViewById(R.id.mFormatInfo).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((TextView)findViewById(R.id.mInfo)).setText(appCodePlayer.getAVFormatInfo());
			}
		});
		findViewById(R.id.mCodecInfo).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((TextView)findViewById(R.id.mInfo)).setText(appCodePlayer.getAVCodecInfo());
			}
		});

		findViewById(R.id.mPrepare).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {

			}
		});
	}

	@Override
	protected void onResume() {
		super.onResume();
		appCodePlayer.prepare();
	}

	private void initPermission(){
        if(PermissionManager.isExternalStorageGranted()){
			ActivityCompat.requestPermissions(this,
					new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,Manifest.permission.READ_EXTERNAL_STORAGE}, 1);
		}
	}
}


