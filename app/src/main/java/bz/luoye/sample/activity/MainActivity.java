package bz.luoye.sample.activity;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZPermissionUtil;
import com.luoye.bzmedia.BZMedia;

import java.io.File;
import java.util.ArrayList;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "bz_MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestPermission();
    }

    @Override
    protected void onResume() {
        super.onResume();
        File fileDir = new File(FilePathUtil.getWorkDir());
        if (!fileDir.exists()) {
            fileDir.mkdirs();
        }
    }


    private boolean requestPermission() {
        ArrayList<String> permissionList = new ArrayList<>();
        if (!BZPermissionUtil.isPermissionGranted(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {
            permissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        }
        if (!BZPermissionUtil.isPermissionGranted(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            permissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        }
        if (!BZPermissionUtil.isPermissionGranted(this, Manifest.permission.CAMERA)) {
            permissionList.add(Manifest.permission.CAMERA);
        }
        if (!BZPermissionUtil.isPermissionGranted(this, Manifest.permission.RECORD_AUDIO)) {
            permissionList.add(Manifest.permission.RECORD_AUDIO);
        }

        String[] permissionStrings = new String[permissionList.size()];
        permissionList.toArray(permissionStrings);

        if (permissionList.size() > 0) {
            BZPermissionUtil.requestPermission(this, permissionStrings, BZPermissionUtil.CODE_REQ_PERMISSION);
            return false;
        } else {
            BZLogUtil.d(TAG, "Have all permissions");
            return true;
        }
    }

    public void test(View view) {
        long startTime = System.currentTimeMillis();
        BZMedia.test();
        BZLogUtil.d(TAG, "test time cost=" + (System.currentTimeMillis() - startTime));
    }

    public void RecorderTest(View view) {
        startActivity(new Intent(this, RecorderTestActivity.class));
    }

    public void OtherTest(View view) {
        startActivity(new Intent(this, OtherTestActivity.class));
    }

    public void AudioPlayerActivity(View view) {
        startActivity(new Intent(this, AudioPlayerActivity.class));
    }
}
