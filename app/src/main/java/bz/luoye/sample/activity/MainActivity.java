package bz.luoye.sample.activity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZFilePathUtil;
import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZPermissionUtil;
import com.luoye.bzmedia.BZMedia;

import java.io.File;

import bz.luoye.sample.R;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "bz_MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        BZPermissionUtil.requestCommonTestPermission(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        File fileDir = new File(BZFilePathUtil.getWorkDir());
        if (!fileDir.exists()) {
            boolean mkdirs = fileDir.mkdirs();
            BZLogUtil.d(TAG, "mkdirs result=" + mkdirs);
        }
    }

    private boolean noPermission() {
        if (!BZPermissionUtil.requestCommonTestPermission(this)) {
            Toast.makeText(this, "Please give App sufficient permissions", Toast.LENGTH_LONG).show();
            return true;
        }
        return false;
    }

    public void test(View view) {
        if (noPermission()) {
            return;
        }
        long startTime = System.currentTimeMillis();
        BZMedia.test();
        BZLogUtil.d(TAG, "time cost=" + (System.currentTimeMillis() - startTime));
    }


    public void RecorderTest(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, RecorderTestActivity.class));
    }

    public void APITest(View view) {
        if (noPermission()) {
            return;
        }
//        startActivity(new Intent(this, APITestActivity.class));
    }

    public void API2Test(View view) {
        if (noPermission()) {
            return;
        }
//        startActivity(new Intent(this, APITest2Activity.class));
    }
}
