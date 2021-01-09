# Building

## Requirements
Building KNG requires the following things:
- a C/C++ compiler
- LLVM build & installed
  - In the future, this will not be required when there is a handwritten backend. For now, KNG requires LLVM



## Running Build Scripts

Navigate to /scripts to view the build scripts. As KNG does not come bundled with an LLVM build (due to platform dependencies etc), you are required to modify the LLVM_DIR inside the build script you are using. Inside either winbuild.bat or unixbuild.sh, modify the -DLLVM_DIR variable to the root of your build LLVM project.