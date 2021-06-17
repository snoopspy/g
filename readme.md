<img src="img/g.png" width="48" height="48"> Library
===

## Build

### Linux
* Run ./deps.sh in command line.
* Install Qt.
* Run the follwing command.
  ```
  make
  make distclean
  ```

### Windows
* Install Qt(MinGW).
* Run the follwing command.
  ```
  mingw32-make -f makefile-wni
  mingw32-make distclean -f makefile-wni
  ```

### Android
* Run the following commands.
  ```
  source ../android-build/android.profile
  ./android-build.sh
  make distclean
  ```
