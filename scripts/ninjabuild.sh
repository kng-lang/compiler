cmake -B ../build -G Ninja -DLLVM_PATH="C:/Program Files (x86)/llvm" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
ninja -C ../build -j 16
$SHELL