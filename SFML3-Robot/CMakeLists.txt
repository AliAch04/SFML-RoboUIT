cmake_minimum_required(VERSION 3.10)
project(MazeRobotSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/core/GameEngine.cpp
    src/core/Maze.cpp
    src/core/Robot.cpp
    src/core/Cell.cpp
    src/algorithms/AStar.cpp
    src/algorithms/Heuristics.cpp
    src/algorithms/MazeGenerator.cpp
    src/ui/Button.cpp
    src/ui/Slider.cpp
    src/ui/TextInput.cpp
    src/utils/SimpleJSON.cpp
)

# Create executable
add_executable(MazeRobotSimulation ${SOURCES})

# Link SFML
target_link_libraries(MazeRobotSimulation sfml-graphics sfml-window sfml-system)

# Copy assets (if any)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()