cmake_minimum_required(VERSION 3.17)
project(MazeRobotSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories("/usr/local/opt/sfml@2/include")
include_directories(.)
include_directories(includes)  

# Link directories
link_directories("/usr/local/opt/sfml@2/lib")

# Explicitly list ALL source files that should be compiled
# excluding SFML3-Robot.cpp and test_learning.cpp
set(SOURCES
    main.cpp
    AStar.cpp
    Button.cpp
    Cell.cpp
    Constants.cpp
    ControlPanelWidget.cpp
    DeepQLearning.cpp
    EditorToolbar.cpp  
    Enums.cpp
    EvolutionaryAStar.cpp
    ExperienceReplay.cpp
    GameEngine.cpp
    Heuristics.cpp
    LearningRobot.cpp
    Maze.cpp
    MazeBrowser.cpp
    MazeBrowserWindow.cpp
    MazeEditor.cpp
    MazeGenerator.cpp
    MazeWidget.cpp
    MetaLearner.cpp
    NeuralNetwork.cpp  
    PerformanceDashboard.cpp
    Point.cpp
    QLearning.cpp
    Robot.cpp
    SimpleJSON.cpp
    Slider.cpp
    SoundManager.cpp
    TextInput.cpp
    TextureManager.cpp
    TrainingVisualizer.cpp
    config.cpp
    logger.cpp
)

# Alternative: if you prefer to keep the GLOB but handle it better:
# file(GLOB ALL_SOURCES *.cpp config.cpp logger.cpp)
# list(FILTER ALL_SOURCES EXCLUDE REGEX "SFML3-Robot\\.cpp$|test_learning\\.cpp$")
# set(SOURCES ${ALL_SOURCES})

add_executable(MazeRobotSimulation ${SOURCES})

target_link_libraries(MazeRobotSimulation
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)

# Copy assets if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Copy maze files if they exist  
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mazes")
    file(COPY mazes DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()