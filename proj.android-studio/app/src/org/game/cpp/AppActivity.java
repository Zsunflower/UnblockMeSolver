/****************************************************************************
Copyright (c) 2015-2017 Chukong Technologies Inc.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package org.game.cpp;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;

import org.cocos2dx.lib.Cocos2dxActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class AppActivity extends Cocos2dxActivity
{
    public static int RESULT_LOAD_IMAGE = 100;
    private static AppActivity _appActiviy;
    private static final int ALL_PERMISSIONS_REQUEST = 100;

    private static final String AD_UNIT_ID = "ca-app-pub-5118564015725949/5530784279";
    private AdView adView;
    private InterstitialAd mInterstitialAd;
    private String cachePath;
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.setEnableVirtualButton(false);
        super.onCreate(savedInstanceState);
        // Workaround in https://stackoverflow.com/questions/16283079/re-launch-of-activity-on-home-button-but-only-the-first-time/16447508
        if (!isTaskRoot())
        {
            // Android launched another instance of the root activity into an existing task
            //  so just quietly finish and go away, dropping the user back into the activity
            //  at the top of the stack (ie: the last state of this task)
            // Don't need to finish it again since it's finished in super.onCreate .
            return;
        }
        // DO OTHER INITIALIZATION BELOW
        checkPermissions();
        cachePath = this.getFilesDir().getAbsolutePath();
        _appActiviy = this;
        extractTemplate();
        setCachePath(cachePath + "/");
        MobileAds.initialize(this, "ca-app-pub-5118564015725949~1731263776");
        loadAd();
    }

    public void loadAd()
    {
        adView = new AdView(this);
        adView.setAdSize(AdSize.SMART_BANNER);
        adView.setAdUnitId(AD_UNIT_ID);

        AdRequest adRequest = new AdRequest.Builder()
                .addTestDevice(AdRequest.DEVICE_ID_EMULATOR)
                .addTestDevice("36EEB4EE860265531C1386E371EF36D1")
                .build();

        adView.loadAd(adRequest);
        adView.setBackgroundColor(Color.BLACK);
        adView.setBackgroundColor(0);
        LinearLayout linearLayout = new LinearLayout(this);
        LinearLayout.LayoutParams adParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        adParams.gravity = Gravity.BOTTOM;
        linearLayout.addView(adView, adParams);
        mFrameLayout.addView(linearLayout);

        mInterstitialAd = new InterstitialAd(this);
        mInterstitialAd.setAdUnitId("ca-app-pub-5118564015725949/4461245187");
        mInterstitialAd.setAdListener(new AdListener() {
            @Override
            public void onAdLoaded() {
                // Code to be executed when an ad finishes loading.
//                callbackOnAdLoaded();
            }

            @Override
            public void onAdFailedToLoad(int errorCode) {
                // Code to be executed when an ad request fails.
//                callbackOnAdFailedToLoad();
                Log.d("Admob", "-------------- onAdFailedToLoad -------------");
            }

            @Override
            public void onAdOpened() {
                // Code to be executed when the ad is displayed.
//                callbackOnAdOpened();
            }

            @Override
            public void onAdLeftApplication() {
                // Code to be executed when the user has left the app.
//                callbackOnAdLeftApplication();
            }

            @Override
            public void onAdClosed() {
                // Code to be executed when when the interstitial ad is closed.
//                callbackOnAdClosed();
            }
        });
        mInterstitialAd.loadAd(new AdRequest.Builder().build());
    }

    public static void chooseFile()
    {
        Log.d("JAVA_AppActivity", "Called from JNI");
        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        _appActiviy.startActivityForResult(i, RESULT_LOAD_IMAGE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == RESULT_LOAD_IMAGE && resultCode == RESULT_OK)
        {
            if (data != null)
            {
                Uri imagePathUri = data.getData();
                if (imagePathUri == null)
                    return;
                String[] filePathColumn = {MediaStore.Images.Media.DATA};
                Cursor cursor = getContentResolver().query(imagePathUri, filePathColumn, null, null, null);
                cursor.moveToFirst();
                int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
                String imagePath = cursor.getString(columnIndex);
                Log.d("JAVA_Activity", imagePath);
                cursor.close();
                solve(imagePath);
            }
        }
    }

    public static native void solve(String gamePath);

    public static native void setCachePath(String gamePath);

    private void extractTemplate()
    {
        String templates[] = {"2blockh.png", "2blockv.png", "3blockh.png", "3blockv.png", "red.png"};
        Log.d("UBMSolver Java cache:", cachePath);
        for (String filename : templates)
        {
            File file = new File(cachePath, filename);
            if (!file.exists())
                try
                {
                    InputStream is = this.getAssets().open("templates/" + filename);
                    int size = is.available();
                    byte[] buffer = new byte[size];
                    is.read(buffer);
                    is.close();

                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(buffer);
                    fos.close();
                } catch (Exception e)
                {
                    throw new RuntimeException(e);
                }
        }
    }

    public void checkPermissions()
    {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, ALL_PERMISSIONS_REQUEST);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults)
    {
        switch (requestCode)
        {
            case (ALL_PERMISSIONS_REQUEST):
            {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
                {
                    Log.d("UBMSolver", "Permissions was granted");
                }
                else
                {
                    checkPermissions();
                }
                break;
            }
            default:
            {
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
                break;
            }
        }
    }

    public static void hideAd()
    {
        _appActiviy.runOnUiThread(new Runnable()
        {

            @Override
            public void run()
            {
                if (_appActiviy.adView.isEnabled())
                    _appActiviy.adView.setEnabled(false);
                if (_appActiviy.adView.getVisibility() != View.INVISIBLE)
                    _appActiviy.adView.setVisibility(View.INVISIBLE);
            }
        });
    }

    public static void showAd()
    {
        _appActiviy.runOnUiThread(new Runnable()
        {

            @Override
            public void run()
            {
                if (!_appActiviy.adView.isEnabled())
                    _appActiviy.adView.setEnabled(true);
                if (_appActiviy.adView.getVisibility() == View.INVISIBLE)
                    _appActiviy.adView.setVisibility(View.VISIBLE);
            }
        });

    }

    public static void showInterstitialAd() {

        _appActiviy.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (_appActiviy.mInterstitialAd.isLoaded()) {
                    _appActiviy.mInterstitialAd.show();
                } else {
                    _appActiviy.mInterstitialAd.loadAd(new AdRequest.Builder().build());
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (adView != null) {
            adView.resume();
        }
    }

    @Override
    protected void onPause() {
        if (adView != null) {
            adView.pause();
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        adView.destroy();
        super.onDestroy();
    }
}