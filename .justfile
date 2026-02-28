
alias r := run
alias b := build

build:
    cmake -S . -B "build" -G Ninja -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Debug
    ninja -C "build"

release:
    cmake -S . -B "build" -G Ninja -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Release
    ninja -C "build"

clean:
    rm -rf ./build

run: build
    build/strike_counter
# LSAN_OPTIONS=suppressions=asan_suppressions.txt build/strike_counter
