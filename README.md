# SA:MP Mobile Client 2.10 (x64) for GTA: San Andreas

# ᐳ Description
> * Arch: `arm64-v8a`, `armeabi-v7a`
> * Type: `Modified SA:MP client` 
> * Security: `https://github.com/ac3ss0r/obfusheader.h` 
> * Status: `Supported`

# ᐳ Features
> * Support `arm64` and `arm32`
> * Built-in OpenGL reflections (cars)
> * WaterShader
> * custom SkyBox
> * disabled `CCorona` (optimization) 
> * Added mini-map icons 
> * Improved multi-touch 
> * Support for the kick button 
> * Compatibility with SA:MP 0.3.7

# ᐳ Credit's
> - **x1y2z** — main author of client 
> - [**kuzia15**](https://github.com/kuzia15) — OpenGL, WaterShader, small improvements
> - [**psychobye**](https://github.com/psychobye) — CSkyBox, small improvements

# ᐳ Pending Tasks

## 🔴 High Priority

### 1. Hook Validation System with User Feedback
> **Status:** `Pending`  
> **Complexity:** `High`  
> **Target:** `Android 11+`

**Problem:**
- If a hook fails, the game may crash later with no clear error
- Current `exit(0)` closes the app without user feedback
- No way to know which hook failed

**Proposed Solution:**
- Create `CHookValidator` class to track all hooks
- Wrapper `InlineHookValidated()` with success/failure logging
- Show user-friendly error dialog via JNI when critical hooks fail
- Option to send error report to server/crashlytics

**Files to modify:**
- `app/src/main/cpp/samp/vendor/armhook/patch.h`
- `app/src/main/cpp/samp/vendor/armhook/patch.cpp`
- `app/src/main/cpp/samp/game/hooks.cpp`
- `app/src/main/java/com/samp/mobile/game/SAMP.java`

---

### 2. W^X Compatible Memory Patching (UnFuck/ReFuck)
> **Status:** `In Planning`  
> **Complexity:** `Medium`  
> **Target:** `Android 15+ / Android 16`

**Problem:**
- Android 15+ enforces W^X (Write XOR Execute) policy
- `mprotect(RWX)` may fail on newer Android versions
- Current `UnFuck()` does not restore execute permission after writing
- ~170+ patches depend on this function

**Proposed Solution:**
- New flow: `RW → Write → cacheflush → RX`
- Add `ReFuck()` function to restore execute permission
- Update `WriteMemory()`, `NOP()`, `RET()`, `Redirect()` to use new flow
- Fallback to RWX for older Android versions

**Architecture:**
```
┌─────────────────────────────────────────────────┐
│              W^X COMPATIBLE FLOW                │
├─────────────────────────────────────────────────┤
│ 1. UnFuck(addr)   → mprotect(RW)               │
│ 2. memcpy()       → Write patch bytes          │
│ 3. cacheflush()   → Sync instruction cache     │
│ 4. ReFuck(addr)   → mprotect(RX)               │
└─────────────────────────────────────────────────┘
```

**Files to modify:**
- `app/src/main/cpp/samp/vendor/armhook/patch.h`
- `app/src/main/cpp/samp/vendor/armhook/patch.cpp`

---

## 🟡 Medium Priority

### 3. Remove exit(0) from getSym()
> **Status:** `Pending`  
> **Complexity:** `Medium`  
> **Depends on:** Task #1

**Problem:**
- `getSym()` calls `exit(0)` if symbol not found
- App closes without any user feedback
- Makes debugging difficult

**Files affected:**
- `app/src/main/cpp/samp/vendor/armhook/patch.h` (getSym, CallFunction, InlineHook)
- `app/src/main/cpp/samp/game/RW/RenderWare.cpp` (48 calls to getSym)

---

## 🟢 Completed

### ✅ Android 15+ Storage Compatibility
> **Completed:** `2026-01`

- Implemented JNI-based storage path setting
- Removed `MANAGE_EXTERNAL_STORAGE` permission
- Removed `requestLegacyExternalStorage` attribute  
- Added null checks for `g_pszStorage` in all file operations
- Updated ShadowHook to v2.0.0

---

<img width="1650" height="720" alt="image" src="https://github.com/user-attachments/assets/20197d82-3046-44e2-a2ad-eea36ea756a5" />
<img width="1650" height="720" alt="image" src="https://github.com/user-attachments/assets/dc99126c-32a7-46c8-8233-474d737ade4c" />
<img width="1650" height="720" alt="image" src="https://github.com/user-attachments/assets/e289bf60-023f-4f5f-a76a-8210f3311ca0" />
