source /opt/intel/oneapi/setvars.sh

which icpx
icpx --version

cmake -S . -B build \
  -DCMAKE_C_COMPILER=icx \
  -DCMAKE_CXX_COMPILER=icpx \
  -DCMAKE_BUILD_TYPE=Debug \
  -DONEAPP_BUILD_CLIENT=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

grep 'CMAKE_CXX_COMPILER:' build/CMakeCache.txt
cmake --build build -j --verbose
ln -sf build/compile_commands.json .
