<img src="img/g.png" width="48" height="48"> Library
===

## Build

### Linux

|Tool|Version|Setup|
|---|---|---|
|Qt|6.5.3|https://download.qt.io|
|g++|13.2.0|sudo apt install g++|


```
./deps.sh
./build-linux.sh
```

### Windows

|Tool|Version|Setup|
|---|---|---|
|Qt|6.5.3|https://download.qt.io|
|mingw|11.2.0|automatically installed with Qt|
|cmake|3.29.0-rc1|https://cmake.org/download|

```
build-win.bat
```

### Android

|Tool|Version|Setup|
|---|---|---|
|Qt|6.5.3|https://download.qt.io|
|JDK|17.0.10|sudo apt install openjdk-17-jdk|
|SDK|12.0|automatically installed with Qt|
|NDK|25.1.8937393|automatically installed with Qt|

```
git clone https://gitlab.com/gilgil/android-build.git ../android-build
source ../android-build/android.profile
./build-android.sh
```

