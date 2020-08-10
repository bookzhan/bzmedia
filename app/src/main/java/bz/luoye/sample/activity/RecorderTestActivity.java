package bz.luoye.sample.activity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;

import bz.luoye.sample.R;

public class RecorderTestActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recorder_test);
    }

    public void Recoder4Camera1(View view) {
        startActivity(new Intent(this, Recorder4Camera1Activity.class));
    }

    public void Recoder4Camera2(View view) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            startActivity(new Intent(this, Recorder4Camera2Activity.class));
        } else {
            BZLogUtil.e("SDK_INT < android.os.Build.VERSION_CODES.LOLLIPOP");
        }

    }

    public void Recoder4TextureActivity(View view) {
        startActivity(new Intent(this, Recorder4TextureActivity.class));
    }

    public void Recoder4BitmapActivity(View view) {
        startActivity(new Intent(this, Recorder4BitmapActivity.class));
    }

}
