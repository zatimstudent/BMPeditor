cmake_minimum_required(VERSION 3.23)
project(untitled2)

cmake_policy(SET CMP0020 NEW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Release)

# Cesta k instalaci Qt
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")


# Nalezení Qt komponent
find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

# Seznam zdrojových souborů
set(SOURCE_FILES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
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

# Vytvoření spustitelného souboru - pro Windows použití WIN32 pro GUI aplikaci
if(WIN32)
        add_executable(untitled2 WIN32 ${SOURCE_FILES})
        # Pro případ, že main.cpp neobsahuje WinMain entry point
        target_link_libraries(untitled2 Qt5::WinMain)
else()
        add_executable(untitled2 ${SOURCE_FILES})
endif()

# Připojení Qt knihoven
target_link_libraries(untitled2
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)

# Automatické kopírování Qt DLL souborů při buildu (pro Windows)
if(WIN32)
        # Kopírování základních Qt knihoven
        add_custom_command(TARGET untitled2 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Qt5::Core>
                $<TARGET_FILE:Qt5::Gui>
                $<TARGET_FILE:Qt5::Widgets>
                $<TARGET_FILE_DIR:untitled2>
        )

        # Kopírování MinGW runtime knihoven z adresáře Qt
        add_custom_command(TARGET untitled2 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "C:/Qt2/5.15.1/mingw81_64/bin/libgcc_s_seh-1.dll"
                "C:/Qt2/5.15.1/mingw81_64/bin/libstdc++-6.dll"
                "C:/Qt2/5.15.1/mingw81_64/bin/libwinpthread-1.dll"
                $<TARGET_FILE_DIR:untitled2>
        )

        # Volitelně můžete přidat i platformní plugin pro správné vykreslování
        add_custom_command(TARGET untitled2 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:untitled2>/platforms"
        )
        add_custom_command(TARGET untitled2 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "C:/Qt2/5.15.1/mingw81_64/plugins/platforms/qwindows.dll"
                "$<TARGET_FILE_DIR:untitled2>/platforms"
        )
endif()