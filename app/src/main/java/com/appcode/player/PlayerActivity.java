package com.appcode.player;

import android.os.Bundle;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.view.SurfaceView;
import android.view.View;
import android.widget.Toast;

public class PlayerActivity extends AppCompatActivity {


	private SurfaceView mSurfaceView;

	private AppCodePlayer appCodePlayer;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_player);

		mSurfaceView = findViewById(R.id.mSurfaceView);

		appCodePlayer = new AppCodePlayer();
		appCodePlayer.setSurfaceView(mSurfaceView);
		appCodePlayer.setDataSource("/sdcard/DCIM/Camera/4a42fe1fa2335c3325142c30ce3af6d1.mp4");

		appCodePlayer.setOnPreparedListener(new AppCodePlayer.OnPreparedListener() {
			@Override
			public void onPrepared() {
				appCodePlayer.start();
				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						Toast.makeText(PlayerActivity.this,"可以开始播放了",Toast.LENGTH_LONG).show();;
					}
				});
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

	@Override
	protected void onPause() {
		super.onPause();
		appCodePlayer.stop();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		appCodePlayer.release();
	}
}
