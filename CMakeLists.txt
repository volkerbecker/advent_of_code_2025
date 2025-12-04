cmake_minimum_required(VERSION 4.0)


# Projektname für die Gesamt-Solution
project(AdventOfCodeProjects LANGUAGES CXX)

# Automatisch den neuesten verfügbaren C++-Standard wählen
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(tools)
