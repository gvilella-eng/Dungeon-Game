cmake_minimum_required(VERSION 3.16)
project(Masmorra LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Funciona com Qt 6 (casa) e Qt 5.15 (empresa): usa o Qt que o kit do Qt Creator apontar.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui)
    set(MASMORRA_QT_LIBS Qt5::Core Qt5::Gui)
else()
    # No Qt 6, QOpenGLWindow / QOpenGLShaderProgram / QOpenGLFunctions_* foram para o modulo OpenGL
    find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL)
    set(MASMORRA_QT_LIBS Qt6::Core Qt6::Gui Qt6::OpenGL)
endif()
message(STATUS "Masmorra: usando Qt ${QT_VERSION}")

file(GLOB_RECURSE MASMORRA_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)

add_executable(Masmorra ${MASMORRA_SOURCES} resources/resources.qrc)
target_include_directories(Masmorra PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(Masmorra PRIVATE ${MASMORRA_QT_LIBS})

# -O2 em todas as configuracoes (o mesher e quente; Debug em -O0 seria inutilizavel)
target_compile_options(Masmorra PRIVATE -O2 -Wall -Wextra -Wno-unused-parameter)
set_target_properties(Masmorra PROPERTIES OUTPUT_NAME "Masmorra")
