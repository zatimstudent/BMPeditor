cmake_minimum_required(VERSION 3.23)
project(untitled2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(untitled2 main.cpp mainwindow.cpp mainwindow.h mainwindow.ui
        customimagewidget.cpp
        customimagewidget.h)
target_link_libraries(untitled2
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )

