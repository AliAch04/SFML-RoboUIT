cmake_minimum_required(VERSION 3.17)
project(MazeRobotSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories("/usr/local/opt/sfml@2/include")
include_directories(.)


link_directories("/usr/local/opt/sfml@2/lib")


file(GLOB SOURCES "*.cpp")


list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/SFML3-Robot.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_learning.cpp")

add_executable(MazeRobotSimulation ${SOURCES}
        MazeBrowser.cpp
        MazeBrowser.h)



target_link_libraries(MazeRobotSimulation
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        sfml-network
)


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()