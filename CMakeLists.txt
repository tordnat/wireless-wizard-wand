cmake_minimum_required(VERSION 3.13.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wireless_wizard_wand)
include_directories(include)

target_sources(app PRIVATE src/ei.cpp)

add_definitions(-DEIDSP_USE_CMSIS_DSP=1
                -DEIDSP_LOAD_CMSIS_DSP_SOURCES=1
                -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1
                -DARM_MATH_LOOPUNROLL)

if(NOT TARGET app)
     message(FATAL_ERROR "Please create a target named 'app' (ex: add_executable(app)) before adding this file")
endif()

include(edge-impulse-sdk/cmake/utils.cmake)
add_subdirectory(edge-impulse-sdk/cmake/zephyr)

RECURSIVE_FIND_FILE_APPEND(MODEL_SOURCE "tflite-model" "*.cpp")
target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE edge-impulse-sdk/classifier)
# add all sources to the project
target_sources(app PRIVATE ${MODEL_SOURCE})
