cmake_minimum_required(VERSION 3.16)
project(Dungeon3DArchitecture VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    3DCore
    3DRender
    3DInput
    3DLogic
    3DExtras
    3DQuick
    3DQuickExtras
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    3DCore
    3DRender
    3DInput
    3DLogic
    3DExtras
    3DQuick
    3DQuickExtras
)

add_executable(Dungeon3DArchitecture
    main.cpp
    resources.qrc
)

target_link_libraries(Dungeon3DArchitecture PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::3DCore
    Qt${QT_VERSION_MAJOR}::3DRender
    Qt${QT_VERSION_MAJOR}::3DInput
    Qt${QT_VERSION_MAJOR}::3DLogic
    Qt${QT_VERSION_MAJOR}::3DExtras
    Qt${QT_VERSION_MAJOR}::3DQuick
    Qt${QT_VERSION_MAJOR}::3DQuickExtras
)