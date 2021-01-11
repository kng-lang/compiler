cmake -B ../build -G "Visual Studio 16" -DLLVM_PATH=$1 -DCMAKE_CXX_FLAGS=-Wall -DCMAKE_BUILD_TYPE=Release
cd ../build
msbuild Release.vcxproj -maxCpuCount:16 /property:Configuration=Release
cd ../scripts
$SHELL