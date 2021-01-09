cmake -B ../build -G "Unix Makefiles" -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-Wall -DCMAKE_BUILD_TYPE=Release
cd ../build
make clean
make -j 16