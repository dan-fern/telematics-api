cmake_minimum_required( VERSION 3.5 )

option( BUILD_TESTS "Build unit tests" OFF )

option( BUILD_COVERAGE_REPORT "Build coverage report" OFF )

option( BUILD_SIM "Build ASP simulator" OFF )

add_definitions( -std=c++11 )

project( telematics-api )

if( BUILD_COVERAGE_REPORT )
        set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage ")
        set(CMAKE_EXE_LINKER_FLAGS "-lgcov ")
endif( BUILD_COVERAGE_REPORT )

include_directories(
        include
        src
        res
        utils
        depends/picosha2/
        depends/json/single_include/nlohmann/
)

file( GLOB HEADERS
        include/*.h
        include/*.hpp
        depends/picosha2/picosha2.h
        depends/json/single_include/nlohmann/json.hpp
)

file( GLOB SRC_LIB
        src/sockethandler.cpp
        src/signalhandler.cpp
        src/remotedevicehandler.cpp
        src/templatehandler.cpp
)

add_library(
        telematics-api-lib
        SHARED
        ${HEADERS}
        ${SRC_LIB}
)

add_executable(
        telematics-api
        src/main.cpp
)

target_link_libraries(
        telematics-api
        PUBLIC
        telematics-api-lib
        pthread
)

if( BUILD_SIM )
        add_subdirectory( utils/asp_simulator )
endif( BUILD_SIM )

if( BUILD_TESTS )
        add_subdirectory( test )
endif( BUILD_TESTS )
