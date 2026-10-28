cmake_minimum_required(VERSION 3.16)
project(Masmorra LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui)

file(GLOB_RECURSE MASMORRA_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)

add_executable(Masmorra ${MASMORRA_SOURCES} resources/resources.qrc)
target_include_directories(Masmorra PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(Masmorra PRIVATE Qt5::Core Qt5::Gui)

# -O2 em todas as configuracoes (o mesher e quente; Debug em -O0 seria inutilizavel)
target_compile_options(Masmorra PRIVATE -O2 -Wall -Wextra -Wno-unused-parameter)
set_target_properties(Masmorra PROPERTIES OUTPUT_NAME "Masmorra")
