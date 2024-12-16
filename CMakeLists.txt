
cmake_minimum_required(VERSION 3.23)
project(matscript)

enable_testing()
add_subdirectory(lib)

add_executable(
    matscript
    src/main.cpp
    src/tokenizer.cpp
    src/token.cpp
    src/parsererror.cpp
    src/vm.cpp
    )

target_compile_features(
    matscript
    PRIVATE
    cxx_std_23
    )

find_package(Threads)
target_link_libraries(
    matscript
    PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    matperf
    )

add_subdirectory(test)

file(
    COPY
    test.msc
    DESTINATION
    .
)

