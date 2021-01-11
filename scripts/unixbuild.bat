cmake -B ../build -G "Unix Makefiles" -DLLVM_PATH=%1 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-Wall -DCMAKE_BUILD_TYPE=Release
cd ../build
make clean
make -j 16
cd ../scripts
$SHELL