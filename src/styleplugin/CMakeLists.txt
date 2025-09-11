cmake_minimum_required(VERSION 3.19)

project(phantomstyleplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt setup
find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# Create the plugin library
add_library(${PROJECT_NAME} SHARED
    phantomstyleplugin.cpp
    phantomstyleplugin.h
    phantomstyleplugin.json

    ../Phantom/phantomcolor.cpp
    ../Phantom/phantomcolor.h
    ../Phantom/phantomstyle.cpp
    ../Phantom/phantomstyle.h
    ../Phantom/phantomtweak.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ../Phantom
)

# Build type suffix for library name
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_debug")

# Set output into the Qt Style plugin path
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG $ENV{QTDIR}/plugins/styles
    RUNTIME_OUTPUT_DIRECTORY_RELEASE $ENV{QTDIR}/plugins/styles
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
