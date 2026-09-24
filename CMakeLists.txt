cmake_minimum_required(VERSION 3.24)

set(ENV{IDF_PATH} "$ENV{IDF_PATH}")
list(APPEND CMAKE_MODULE_PATH "$ENV{IDF_PATH}/tools/cmake")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_po33_ko)