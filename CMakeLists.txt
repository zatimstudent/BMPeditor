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
        customimagewidget.h
        Filters/Filter.h
        Filters/InvertFilter.cpp
        Filters/InvertFilter.h
        Filters/RotateFilter.cpp
        Filters/RotateFilter.h
        Filters/FlipFilter.cpp
        Filters/FlipFilter.h
        styles.h
        Image.cpp
        Image.h
)
target_link_libraries(untitled2
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )

