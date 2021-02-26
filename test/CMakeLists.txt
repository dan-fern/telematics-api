cmake_minimum_required( VERSION 3.5 )

add_definitions( -std=c++11 )

add_subdirectory( gtest )

project( telematics-api-tests )

file( GLOB HEADERS "*.hpp")
file( GLOB SRC "*.cpp" )

if( BUILD_COVERAGE_REPORT )
        set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage ")
        set(CMAKE_EXE_LINKER_FLAGS "-lgcov ")
endif( BUILD_COVERAGE_REPORT )

include_directories("${CMAKE_SOURCE_DIR}/utils/asp_simulator")

add_executable(
        telematics-api-tests
        ${HEADERS}
        ${SRC}
        "${CMAKE_SOURCE_DIR}/utils/asp_simulator/aspm.cpp"
)

find_library( API_LIB
    NAMES telematics-api-lib
    PATHS ../build
)

target_link_libraries(
        telematics-api-tests
        PUBLIC
        ${API_LIB}
        pthread
        gtest
)
