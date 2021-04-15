package bz.luoye.sample.activity;

import static android.Manifest.permission.CAMERA;
import static android.Manifest.permission.READ_CONTACTS;
import static android.Manifest.permission.READ_EXTERNAL_STORAGE;
import static android.Manifest.permission.RECORD_AUDIO;
import static android.Manifest.permission.WRITE_EXTERNAL_STORAGE;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZPermissionUtil;
import com.luoye.bzmedia.BZMedia;

import java.io.File;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "bz_MainActivity";
    private ImageView iv_test;

    public static final String[] mustHavePermissions =
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.R ?
                    new String[] {CAMERA, RECORD_AUDIO, READ_CONTACTS, READ_EXTERNAL_STORAGE} :
                    new String[] {CAMERA, RECORD_AUDIO, READ_CONTACTS, READ_EXTERNAL_STORAGE, WRITE_EXTERNAL_STORAGE};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        iv_test = findViewById(R.id.iv_test);
        requestPermission();

    }

    @Override
    protected void onResume() {
        super.onResume();
        File fileDir = new File(FilePathUtil.getReadWorkDir());
        if (!fileDir.exists()) {
            fileDir.mkdirs();
        }
    }


    private boolean requestPermission() {

        if (mustHavePermissions.length > 0) {
            BZPermissionUtil.requestPermission(this, mustHavePermissions, BZPermissionUtil.CODE_REQ_PERMISSION);
            return false;
        } else {
            BZLogUtil.d(TAG, "Have all permissions");
            return true;
        }
    }

    public void test(View view) {
        long startTime = System.currentTimeMillis();
        BZMedia.test();
        BZLogUtil.d(TAG, "time cost=" + (System.currentTimeMillis() - startTime));

    }

    public void RecorderTest(View view) {
        startActivity(new Intent(this, RecorderTestActivity.class));
    }

    public void APITest(View view) {
//        startActivity(new Intent(this, APITestActivity.class));
    }

    public void API2Test(View view) {
//        startActivity(new Intent(this, APITest2Activity.class));
    }
}
