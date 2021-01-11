cmake -B ../build -G "Visual Studio 16" -DLLVM_PATH="C:/Program Files (x86)/llvm" -DCMAKE_CXX_FLAGS=-Wall -DCMAKE_BUILD_TYPE=Release
cd ../build
msbuild Release.vcxproj -maxCpuCount:16 /property:Configuration=Release
cd ../scripts
$SHELL