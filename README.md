# SA:MP Mobile Client 2.10 (Android)

Modified Android SA:MP (GTA: San Andreas) client, built with **Android NDK (C++) + JNI + Java** and supporting **ARM32/ARM64**.

## Overview

- **ABIs**: `armeabi-v7a`, `arm64-v8a`
- **Application ID**: `com.samp.mobile`
- **Native build**: CMake (NDK)
- **Status**: supported
- **Compatibility**: SA:MP `0.3.7`

## Key features

- **ARM32 + ARM64 support**
- **OpenGL reflections (vehicles)**
- **WaterShader**
- **Custom SkyBox**
- **Optimizations**: disables `CCorona`
- **UI/controls**: improved multi-touch, kick button support
- **Map**: mini-map icons

## Requirements

- **Android Studio** (recommended)
- **Android SDK**: `compileSdk 36` / `targetSdk 36` / `minSdk 26`
- **NDK**: `26.2.11394342`
- **Android Gradle Plugin**: `com.android.tools.build:gradle:8.4.0`

## Build (Debug)

> Note: this project uses a hybrid build (Java + C++ via CMake). The first build may take longer due to toolchain/NDK setup.

### Android Studio

- Open the project in Android Studio
- Select the **Debug** variant
- Build/Run as usual

### Command line

From the repository root:

```bash
./gradlew assembleDebug
```

On Windows (PowerShell):

```powershell
.\gradlew assembleDebug
```

## Project structure (high level)

- `app/`: Android module (Java/Kotlin, resources, and app Gradle config)
- `app/src/main/cpp/`: native code (C/C++), JNI, and CMake
- `gradle/` and `gradlew*`: Gradle wrapper

## Roadmap (short)

- **Hook validation with user feedback** (avoid “silent” crashes when a hook fails; show a user-friendly error via JNI)
- **Remove `exit(0)` from `getSym()`** (return errors/telemetry instead of closing the app)

## Credits

- **x1y2z** — main author of the client
- [**kuzia15**](https://github.com/kuzia15) — OpenGL, WaterShader, and improvements
- [**psychobye**](https://github.com/psychobye) — CSkyBox and improvements

## License

See `LICENSE`.

## Disclaimer

This repository is intended for educational and engineering purposes. **GTA: San Andreas** and related trademarks belong to their respective owners.

## Screenshots

<img width="1650" height="720" alt="image" src="https://github.com/user-attachments/assets/20197d82-3046-44e2-a2ad-eea36ea756a5" />
<img width="1650" height="720" alt="image" src="https://github.com/user-attachments/assets/dc99126c-32a7-46c8-8233-474d737ade4c" />
<img width="1650" height="720" alt="image" src="https://github.com/user-attachments/assets/e289bf60-023f-4f5f-a76a-8210f3311ca0" />
