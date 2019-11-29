package com.appcode.player;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.appcode.utils.PermissionManager;

public class MainActivity extends AppCompatActivity {


	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		initPermission();

		findViewById(R.id.mProtocolInfo).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((TextView)findViewById(R.id.mInfo)).setText(AppCodePlayer.getInstance().getUrlProtocolInfo());
			}
		});
		findViewById(R.id.mFormatInfo).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((TextView)findViewById(R.id.mInfo)).setText(AppCodePlayer.getInstance().getAVFormatInfo());
			}
		});
		findViewById(R.id.mCodecInfo).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((TextView)findViewById(R.id.mInfo)).setText(AppCodePlayer.getInstance().getAVCodecInfo());
			}
		});
		findViewById(R.id.mStartPlay).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Intent intent = new Intent(MainActivity.this,PlayerActivity.class);
				startActivity(intent);
			}
		});
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	private void initPermission(){
        if(PermissionManager.isExternalStorageGranted()){
			ActivityCompat.requestPermissions(this,
					new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,Manifest.permission.READ_EXTERNAL_STORAGE}, 1);
		}
	}
}


