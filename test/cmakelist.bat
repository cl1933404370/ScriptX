mkdir Build
cd Build
cmake -G "Visual Studio 17 2022" -Build ..\ -DCMAKE_TOOLCHAIN_FILE=D:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
pause