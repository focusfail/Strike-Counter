
build:
    cmake -S . -B "build" -G Ninja -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Debug
    ninja -C "build" -j 16

build_release:
    cmake -S . -B "build" -G Ninja -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Release
    ninja -C "build" -j 16

clean:
    rm -rf ./build

run:
    LSAN_OPTIONS=suppressions=asan_suppressions.txt build/strike_counter
