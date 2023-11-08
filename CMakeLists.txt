cmake_minimum_required(VERSION 3.5)

project(receiver LANGUAGES CXX C)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -g -O0")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PkgConfig REQUIRED)
find_package(RtlSdr REQUIRED)

if(NOT RTLSDR_FOUND)
    message(FATAL_ERROR "RTL-SDR required")
endif()

file(GLOB SRC include/${PROJECT_NAME}/*.h source/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC fftw3 rtlsdr dsp)
target_link_libraries(${PROJECT_NAME} INTERFACE base)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

include("unittest.cmake")
