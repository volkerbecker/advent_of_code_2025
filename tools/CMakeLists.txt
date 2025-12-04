# Beispiel: day1/CMakeLists.txt

cmake_minimum_required(VERSION 4.0)

# Projektname (exe heißt gleich wie der Ordner)
project(tools LANGUAGES CXX)

# Alle .cpp und .h Dateien automatisch einsammeln
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
