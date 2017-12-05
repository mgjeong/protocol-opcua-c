cmake -H. -Bbuild
cmake --build build -- -j3

cp build/libopcua-adapter.so example/
