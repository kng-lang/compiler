cd ../
cmake -B ./build -G Ninja -DLLVM_PATH=$1 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
ninja -C ./build -j 16
$SHELL