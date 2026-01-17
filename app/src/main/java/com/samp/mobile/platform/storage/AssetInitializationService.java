package com.samp.mobile.platform.storage;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class AssetInitializationService {
    private static final String TAG = "AssetInitialization";
    private static final String PREF_NAME = "asset_initialization";
    private static final String PREF_KEY_INITIALIZED = "initialized";
    
    public static boolean isInitialized(Context context) {
        if (context == null) {
            return false;
        }
        
        SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        return prefs.getBoolean(PREF_KEY_INITIALIZED, false);
    }
    
    public static synchronized boolean initializeAssets(Context context) {
        if (context == null) {
            Log.e(TAG, "initializeAssets: Context is null");
            return false;
        }
        
        if (isInitialized(context)) {
            Log.i(TAG, "initializeAssets: Assets already initialized");
            return true;
        }
        
        AssetStorageManager storageManager = AssetStorageManager.getInstance(context);
        String basePath = storageManager.getBasePath();
        
        Log.i(TAG, "initializeAssets: Starting initialization. Base path: " + basePath);
        
        try {
            String[] assetFiles = context.getAssets().list("");
            if (assetFiles == null || assetFiles.length == 0) {
                Log.w(TAG, "initializeAssets: No assets found in APK");
                markAsInitialized(context);
                return true;
            }
            
            int copiedCount = 0;
            int failedCount = 0;
            
            for (String assetPath : assetFiles) {
                if (copyAssetRecursive(context, assetPath, basePath, "")) {
                    copiedCount++;
                } else {
                    failedCount++;
                    Log.e(TAG, "initializeAssets: Failed to copy asset: " + assetPath);
                }
            }
            
            Log.i(TAG, "initializeAssets: Completed. Copied: " + copiedCount + ", Failed: " + failedCount);
            
            if (failedCount == 0 || copiedCount > 0) {
                markAsInitialized(context);
                return true;
            }
            
            return false;
        } catch (IOException e) {
            Log.e(TAG, "initializeAssets: IOException", e);
            return false;
        }
    }
    
    private static boolean copyAssetRecursive(Context context, String assetPath, String basePath, String relativePath) {
        try {
            String fullPath = relativePath.isEmpty() ? assetPath : relativePath + "/" + assetPath;
            
            InputStream is = context.getAssets().open(fullPath);
            if (is == null) {
                String[] list = context.getAssets().list(fullPath);
                if (list != null && list.length > 0) {
                    is.close();
                    File dir = new File(basePath + fullPath);
                    if (!dir.exists()) {
                        if (!dir.mkdirs()) {
                            Log.e(TAG, "copyAssetRecursive: Failed to create directory: " + dir.getAbsolutePath());
                            return false;
                        }
                    }
                    
                    boolean allSuccess = true;
                    for (String item : list) {
                        if (!copyAssetRecursive(context, item, basePath, fullPath)) {
                            allSuccess = false;
                        }
                    }
                    return allSuccess;
                }
                return false;
            }
            
            File outputFile = new File(basePath + fullPath);
            File parentDir = outputFile.getParentFile();
            
            if (parentDir != null && !parentDir.exists()) {
                if (!parentDir.mkdirs()) {
                    Log.e(TAG, "copyAssetRecursive: Failed to create parent directory: " + parentDir.getAbsolutePath());
                    is.close();
                    return false;
                }
            }
            
            FileOutputStream fos = new FileOutputStream(outputFile);
            byte[] buffer = new byte[8192];
            int bytesRead;
            
            while ((bytesRead = is.read(buffer)) != -1) {
                fos.write(buffer, 0, bytesRead);
            }
            
            fos.flush();
            fos.close();
            is.close();
            
            return true;
        } catch (IOException e) {
            Log.e(TAG, "copyAssetRecursive: IOException for " + assetPath, e);
            return false;
        }
    }
    
    private static void markAsInitialized(Context context) {
        SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        prefs.edit().putBoolean(PREF_KEY_INITIALIZED, true).apply();
    }
}
