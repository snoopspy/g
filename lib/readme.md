### Linux

|Tool|Version|Setup|
|---|---|---|
|googletest|1.14.0-1|sudo apt install libgtest-dev|

### Windows

|Tool|Version|Setup|
|---|---|---|
|googletest|v1.14.0|http://github.com/google/googletest|

```
git clone http://github.com/google/googletest.git
cd googletest/googletest
cmake .. -DCMAKE_C_COMPILER="gcc" -DCMAKE_CPP_COMPILER="g++" -DCMAKE_MAKE_PROGRAM="mingw32-make" -G "MinGW Makefiles"
mingw32-make
```

