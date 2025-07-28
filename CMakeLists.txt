cmake_minimum_required(VERSION 3.12)
project(adsl LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


add_library(adsl SHARED
    src/adsl.cpp
    src/adsl_api.cpp
)


target_include_directories(adsl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# windows specific settings :
set_target_properties(adsl PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)