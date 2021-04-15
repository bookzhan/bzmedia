package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import bz.luoye.sample.R;
import bz.luoye.sample.views.PathMeasureDemoView;

public class PathMeasureDemoActivity extends AppCompatActivity {

    private PathMeasureDemoView mDemoView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_path_measure_demo);
        mDemoView = findViewById(R.id.demo_view);
    }

    public void Next(View view) {
        mDemoView.invalidate();
    }
}