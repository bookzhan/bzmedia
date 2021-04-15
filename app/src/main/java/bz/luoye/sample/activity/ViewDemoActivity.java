package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import bz.luoye.sample.R;

public class ViewDemoActivity extends AppCompatActivity {

    private View mMRlContent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_view_demo);
        mMRlContent = findViewById(R.id.rl_content);
        mMRlContent.setPivotX(0);
        mMRlContent.setPivotY(0);
    }

    public void test(View view) {
        float scale = 2.5f;
        float offSetX = (mMRlContent.getWidth() * scale - mMRlContent.getWidth()) / 2;
        float offSetY = (mMRlContent.getHeight() * scale - mMRlContent.getHeight()) / 2;
        mMRlContent.setScaleX(scale);
        mMRlContent.setScaleY(scale);
        mMRlContent.setTranslationX(-offSetX);
        mMRlContent.setTranslationY(-offSetY);
    }
}