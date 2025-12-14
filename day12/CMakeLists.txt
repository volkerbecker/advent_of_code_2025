# Beispiel: day1/CMakeLists.txt

cmake_minimum_required(VERSION 4.0)

# Projektname (exe heiﬂt gleich wie der Ordner)
project(day12 LANGUAGES CXX)

# Alle .cpp und .h Dateien automatisch einsammeln
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE tools)
