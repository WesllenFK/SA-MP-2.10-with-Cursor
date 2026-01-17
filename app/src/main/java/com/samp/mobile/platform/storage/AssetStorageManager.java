package com.samp.mobile.platform.storage;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class AssetStorageManager {
    private static final String TAG = "AssetStorageManager";
    private static AssetStorageManager instance;
    private static final Object lock = new Object();
    
    private Context context;
    private String basePath;
    
    private AssetStorageManager(Context context) {
        if (context == null) {
            throw new IllegalArgumentException("Context cannot be null");
        }
        this.context = context.getApplicationContext();
        
        File filesDir = this.context.getFilesDir();
        if (filesDir == null) {
            throw new IllegalStateException("Cannot access files directory");
        }
        
        this.basePath = filesDir.getAbsolutePath() + "/";
    }
    
    public static AssetStorageManager getInstance(Context context) {
        if (instance == null) {
            synchronized (lock) {
                if (instance == null) {
                    instance = new AssetStorageManager(context);
                }
            }
        }
        return instance;
    }
    
    public static AssetStorageManager getInstance() {
        if (instance == null) {
            throw new IllegalStateException("AssetStorageManager not initialized. Call getInstance(Context) first.");
        }
        return instance;
    }
    
    public synchronized String getBasePath() {
        return basePath;
    }
    
    public synchronized boolean fileExists(String path) {
        if (path == null || path.isEmpty()) {
            return false;
        }
        
        if (!isPathValid(path)) {
            Log.e(TAG, "Invalid path: " + path);
            return false;
        }
        
        File file = new File(basePath + path);
        return file.exists() && file.isFile();
    }
    
    public synchronized byte[] readFile(String path) {
        if (path == null || path.isEmpty()) {
            Log.e(TAG, "readFile: path is null or empty");
            return null;
        }
        
        if (!isPathValid(path)) {
            Log.e(TAG, "readFile: Invalid path: " + path);
            return null;
        }
        
        File file = new File(basePath + path);
        if (!file.exists() || !file.isFile()) {
            Log.e(TAG, "readFile: File does not exist: " + path);
            return null;
        }
        
        FileInputStream fis = null;
        try {
            fis = new FileInputStream(file);
            int size = fis.available();
            if (size <= 0) {
                Log.e(TAG, "readFile: File size is invalid: " + size);
                return null;
            }
            
            byte[] data = new byte[size];
            int bytesRead = fis.read(data);
            if (bytesRead != size) {
                Log.e(TAG, "readFile: Read size mismatch. Expected: " + size + ", Got: " + bytesRead);
                return null;
            }
            
            return data;
        } catch (IOException e) {
            Log.e(TAG, "readFile: IOException reading file: " + path, e);
            return null;
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                    Log.e(TAG, "readFile: Error closing FileInputStream", e);
                }
            }
        }
    }
    
    public synchronized boolean writeFile(String path, byte[] data) {
        if (path == null || path.isEmpty()) {
            Log.e(TAG, "writeFile: path is null or empty");
            return false;
        }
        
        if (!isPathValid(path)) {
            Log.e(TAG, "writeFile: Invalid path: " + path);
            return false;
        }
        
        if (data == null) {
            Log.e(TAG, "writeFile: data is null");
            return false;
        }
        
        File file = new File(basePath + path);
        File parentDir = file.getParentFile();
        
        if (parentDir != null && !parentDir.exists()) {
            if (!parentDir.mkdirs()) {
                Log.e(TAG, "writeFile: Failed to create parent directories: " + parentDir.getAbsolutePath());
                return false;
            }
        }
        
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(file, false);
            fos.write(data);
            fos.flush();
            return true;
        } catch (IOException e) {
            Log.e(TAG, "writeFile: IOException writing file: " + path, e);
            return false;
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    Log.e(TAG, "writeFile: Error closing FileOutputStream", e);
                }
            }
        }
    }
    
    public synchronized byte[] readAsset(String assetPath) {
        if (assetPath == null || assetPath.isEmpty()) {
            Log.e(TAG, "readAsset: assetPath is null or empty");
            return null;
        }
        
        InputStream is = null;
        try {
            is = context.getAssets().open(assetPath);
            int size = is.available();
            if (size <= 0) {
                Log.e(TAG, "readAsset: Asset size is invalid: " + size);
                return null;
            }
            
            byte[] data = new byte[size];
            int bytesRead = is.read(data);
            if (bytesRead != size) {
                Log.e(TAG, "readAsset: Read size mismatch. Expected: " + size + ", Got: " + bytesRead);
                return null;
            }
            
            return data;
        } catch (IOException e) {
            Log.e(TAG, "readAsset: IOException reading asset: " + assetPath, e);
            return null;
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    Log.e(TAG, "readAsset: Error closing InputStream", e);
                }
            }
        }
    }
    
    public synchronized boolean assetExists(String assetPath) {
        if (assetPath == null || assetPath.isEmpty()) {
            return false;
        }
        
        InputStream is = null;
        try {
            is = context.getAssets().open(assetPath);
            return true;
        } catch (IOException e) {
            return false;
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                }
            }
        }
    }
    
    private boolean isPathValid(String path) {
        if (path.contains("../") || path.contains("..\\")) {
            return false;
        }
        
        if (path.startsWith("/data/") || path.startsWith("/sdcard/") || path.startsWith("/storage/")) {
            return false;
        }
        
        return true;
    }
}
