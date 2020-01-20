# cmake -DCMAKE_BUILD_TYPE=Debug
# cmake -DCMAKE_BUILD_TYPE=Release

cmake_minimum_required (VERSION 3.1)
project (CollageMaker)

set( CMAKE_CONFIGURATION_TYPES  "Debug;Release;Profile;Gperf;Perf" CACHE STRING "" FORCE )

list(APPEND CMAKE_CXX_FLAGS "-std=c++2a -Werror")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} -pg")
set(CMAKE_CXX_FLAGS_GPERF "${CMAKE_CXX_FLAGS_RELEASE} -DWITHGPERFTOOLS -g")
# not sure if -g option helps
set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE} -g")

file(GLOB SOURCE_FILES "./src/*.cpp")
file(GLOB HEADER_FILES "./include/*.hpp")
file(GLOB TEST_FILES "./test/*")
file(GLOB APP_FILES "./app/*")


include_directories("./include")
# for ant library probably
include_directories("/usr/local/include")
include_directories("/usr/include")


find_library(ANT_LIBRARY ant)
find_library(GTEST_MAIN_LIBRARY gtest_main)
find_library(GTEST_LIBRARY gtest)


add_library(CollageMaker STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(CollageMaker ${ANT_LIBRARY})

if (CMAKE_BUILD_TYPE MATCHES Gperf)
    target_link_libraries(CollageMaker profiler)
endif()

add_executable(CollageMakerApp ${APP_FILES})
target_link_libraries(CollageMakerApp 
    ${ANT_LIBRARY} 
    CollageMaker)

add_executable(CollageMakerTest ${TEST_FILES})
target_link_libraries(CollageMakerTest
    ${GTEST_MAIN_LIBRARY} 
    ${GTEST_LIBRARY} 
    ${ANT_LIBRARY} 
    CollageMaker)
    