package bz.luoye.sample.activity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import bz.luoye.sample.R;

public class RecorderTestActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recorder_test);
    }

    public void Recoder4Camera1(View view) {
        startActivity(new Intent(this, Recoder4Camera1Activity.class));
    }
}
