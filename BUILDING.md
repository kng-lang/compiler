# Building

## Requirements
Building KNG requires the following things:
- CMake
- a C/C++ compiler (by default clang is used, if you want to use gcc/g++ or mingw etc, modify the build scripts)
- LLVM build & installed
  - In the future, this will not be required when there is a handwritten backend. For now, KNG requires LLVM

## Build Scripts

To build KNG for your buildsystem/platform, navigate to the scripts directory. There are various scripts corresponding to different platforms and build systems, choose the script accordingly (msbuild, ninja or make). Each script requires an argument which is the path to your build LLVM project (containing the library files, headers etc). (Note, you can also change the compiler inside the scripts, by default, clang is used)

```
winbuild.bat "C:/Program Files (x86)/llvm"
```

The above example will generate a visual studio project, then build the project using msbuild.