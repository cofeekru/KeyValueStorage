cmake_minimum_required(VERSION 3.10)
project(KVStorage)

set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)

add_executable(kvstorage_test tests/tests.cpp)


target_link_libraries(kvstorage_test
    gtest_main
    gtest
    pthread 
)

enable_testing()
include(GoogleTest)  
add_test(NAME kvstorage_test COMMAND kvstorage_test)
