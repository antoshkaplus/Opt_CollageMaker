# cmake -DCMAKE_BUILD_TYPE=Debug
# cmake -DCMAKE_BUILD_TYPE=Release

cmake_minimum_required (VERSION 3.1)
project (CollageMaker)


list(APPEND CMAKE_CXX_FLAGS "-std=c++1y")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -d -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

set(BinDir "../bin")
set(LibDir "../lib")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BinDir} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LibDir} )
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BinDir} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LibDir} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


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


add_executable(CollageMakerApp ${APP_FILES})
target_link_libraries(CollageMakerApp 
    ${ANT_LIBRARY} 
    CollageMaker)

add_executable(CollageMakerTest ${TEST_FILES})
target_include_directories(CollageMakerTest PUBLIC "/usr/local/include")
target_link_libraries(CollageMakerTest 
    ${GTEST_MAIN_LIBRARY} 
    ${GTEST_LIBRARY} 
    ${ANT_LIBRARY} 
    CollageMaker)
    