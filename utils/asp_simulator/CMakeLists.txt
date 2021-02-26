cmake_minimum_required( VERSION 3.5 )

add_definitions( -std=c++11 )

project( telematics-api-sim )

file( GLOB HEADERS "*.hpp" )

add_library(
    aspm
    SHARED
    ${HEADERS}
    "aspm.cpp"
)

add_executable(
        telematics-api-sim
        ${HEADERS}
        "main.cpp"
)

target_link_libraries(
    aspm
    PUBLIC
    telematics-api-lib
)

target_link_libraries(
        telematics-api-sim
        PUBLIC
        telematics-api-lib
        aspm
        pthread
)
