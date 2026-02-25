# SFML Maze Robot Solver

[![SFML](https://img.shields.io/badge/SFML-2.5%2B-green.svg)](https://www.sfml-dev.org/)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Windows](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
[![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)](https://visualstudio.microsoft.com/)

A sophisticated maze-solving robot simulation built with C++ and SFML. Features multiple pathfinding algorithms, reinforcement learning, and an interactive maze editor.

![Robot Maze Simulation](SFML3-Robot/assets/logo.png)

## Table of Contents
- [Features](#-features)
- [Project Structure](#-project-structure)
- [Prerequisites](#-prerequisites)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Usage Guide](#-usage-guide)
- [Algorithms](#-algorithms)
- [Building from Source](#-building-from-source)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

## Features

- **Multiple Pathfinding Algorithms**
  - A* (A-Star) with Manhattan heuristic
  - Q-Learning reinforcement learning
  - Deep Q-Learning with neural networks
  - Evolutionary A* for adaptive pathfinding
  - Meta-Learning for algorithm selection

- **Interactive Maze Editor**
  - Create and edit custom mazes
  - Set start (S) and end (E) points
  - Add walls (#) and empty spaces (.)
  - Real-time maze generation

- **Visual Learning**
  - Watch robots learn in real-time
  - Visualize explored cells during search
  - Display optimal paths
  - Performance metrics dashboard

- **Audio Feedback**
  - Background music
  - Click sounds for interactions
  - Robot movement sounds

- **Customizable Settings**
  - Adjustable robot speed
  - Cell size configuration
  - Toggle visual elements
  - Save/Load maze configurations

## Project Structure

```
SFML3-Robot/
├── Algorithm/                 # Pathfinding & learning algorithms
│   ├── include/               # Header files
│   │   ├── AStar.h
│   │   ├── QLearning.h
│   │   ├── DeepQLearning.h
│   │   ├── EvolutionaryAStar.h
│   │   ├── NeuralNetwork.h
│   │   └── ...
│   └── src/                   # Source files
│       ├── AStar.cpp
│       ├── QLearning.cpp
│       └── ...
│
├── Engine/                    # Core game engine
│   ├── include/
│   │   ├── GameEngine.h
│   │   ├── Robot.h
│   │   ├── LearningRobot.h
│   │   └── ...
│   └── src/
│       ├── GameEngine.cpp
│       ├── main.cpp
│       └── ...
│
├── Maze/                      # Maze representation
│   ├── include/
│   │   ├── Maze.h
│   │   ├── Cell.h
│   │   └── Point.h
│   └── src/
│       ├── Maze.cpp
│       └── ...
│
├── Editor/                    # Maze editor tools
│   ├── include/
│   │   ├── MazeEditor.h
│   │   └── EditorToolbar.h
│   └── src/
│       ├── MazeEditor.cpp
│       └── EditorToolbar.cpp
│
├── UI/                        # User interface components
│   ├── include/
│   │   ├── Button.h
│   │   ├── Slider.h
│   │   ├── TextInput.h
│   │   └── ...
│   └── src/
│       ├── Button.cpp
│       ├── Slider.cpp
│       └── ...
│
├── Audio/                     # Sound management
│   ├── include/
│   │   └── SoundManager.h
│   └── src/
│       └── SoundManager.cpp
│
├── File/                      # File I/O operations
│   ├── include/
│   │   ├── SimpleJSON.h
│   │   └── config.h
│   └── src/
│       ├── SimpleJSON.cpp
│       └── config.cpp
│
├── Texture/                   # Texture management
│   ├── include/
│   │   └── TextureManager.h
│   └── src/
│       └── TextureManager.cpp
│
├── assets/                    # Game assets
│   ├── logo.png
│   ├── sounds/
│   │   ├── background_music.wav
│   │   └── click.wav
│   └── textures/
│       ├── floor.png
│       ├── wall.png
│       └── robot.PNG
│
├── mazes/                     # Maze JSON files
│   ├── maze1.json
│   ├── maze2.json
│   └── ...
│
├── config.txt                 # Configuration file
├── SFML3-Robot.sln            # Visual Studio solution
└── README.md                  # This file
```

## Prerequisites

### Required Software
- **Windows 10/11** (64-bit or 32-bit)
- **Visual Studio Community 2022** (or higher)
- **Git** (for cloning the repository)

### Required Libraries
- **SFML 2.5.1** (Simple and Fast Multimedia Library)

## Installation

### Step 1: Clone the Repository
```bash
git clone https://github.com/AliAch04/SFML-RoboUIT.git
cd SFML-RoboUIT/SFML3-Robot
```

### Step 2: Download SFML
1. Go to [SFML Downloads](https://www.sfml-dev.org/download.php)
2. Download **SFML 2.5.1 - Visual C++ 17 (2022) - 32-bit**
3. Extract to a location like `C:\SFML-2.5.1`

### Step 3: Configure Visual Studio

#### Option A: Automatic Configuration (Recommended)
1. Open `SFML3-Robot.sln` in Visual Studio 2022
2. Right-click the project in Solution Explorer → **Properties**
3. Go to **C/C++ → General → Additional Include Directories**
4. Add: `C:\SFML-2.5.1\include`
5. Go to **Linker → General → Additional Library Directories**
6. Add: `C:\SFML-2.5.1\lib`
7. Go to **Linker → Input → Additional Dependencies**
   - For Debug: Add `sfml-graphics-d.lib;sfml-window-d.lib;sfml-system-d.lib;sfml-audio-d.lib`
   - For Release: Add `sfml-graphics.lib;sfml-window.lib;sfml-system.lib;sfml-audio.lib`

#### Option B: Using the Provided .vcxproj
The project file already includes SFML configuration. You just need to:
1. Ensure SFML is installed at `C:\SFML-2.5.1`
2. If installed elsewhere, update paths in project properties

### Step 4: Copy SFML DLLs
Copy these DLLs from `C:\SFML-2.5.1\bin` to your project's output directory (`Debug/` or `Release/`):
- `sfml-graphics-2.dll`
- `sfml-window-2.dll`
- `sfml-system-2.dll`
- `sfml-audio-2.dll`
- `openal32.dll`

## Configuration

### config.txt
Edit `config.txt` to customize default settings:

```ini
# Window settings
WINDOW_WIDTH=800
WINDOW_HEIGHT=600
FULLSCREEN=false

# Robot settings
ROBOT_SPEED=0.3
DEFAULT_ALGORITHM=ASTAR

# Display settings
CELL_SIZE=40
SHOW_EXPLORED=true
SHOW_PATH=true

# Learning parameters
LEARNING_RATE=0.1
DISCOUNT_FACTOR=0.9
EXPLORATION_RATE=0.2
```

### Maze Files
Mazes are stored as JSON in the `mazes/` folder:
```json
{
  "name": "My Maze",
  "width": 10,
  "height": 10,
  "layout": [
    "##########",
    "#S...#...#",
    "###.####.#",
    "#...#....#",
    "#.###.##.#",
    "#.#....#.#",
    "#.####.#.#",
    "#......#E#",
    "##########"
  ]
}
```

## Usage Guide

### Building the Project
1. Open `SFML3-Robot.sln` in Visual Studio 2022
2. Select **Debug** or **Release** configuration
3. Select **x86** platform (32-bit)
4. Press `F7` or select **Build → Build Solution**

### Running the Application
- Press `F5` to run with debugging
- Press `Ctrl+F5` to run without debugging

### Main Menu Controls
- **START** - Begin simulation with default maze
- **OPTIONS** - Adjust settings (speed, cell size, visual toggles)
- **EXIT** - Close application

### Game Screen Controls

#### Mouse Controls
- Click buttons in control panel
- Click text inputs to edit maze name/dimensions
- Use sliders in options menu

#### Keyboard Shortcuts
- `R` - Reload default maze
- `Escape` - Return to main menu
- `Tab` - Cycle through text inputs

#### Control Panel Buttons
| Button | Function |
|--------|----------|
| **Zoom+** | Increase cell size |
| **Zoom-** | Decrease cell size |
| **Generate** | Create random solvable maze |
| **Run/Pause** | Start or pause robot movement |
| **Tester** | Check if maze is solvable |
| **Sauver** | Save current maze to JSON |
| **Resize** | Apply new dimensions |
| **Back** | Return to main menu |

### Creating Custom Mazes
1. Enter maze name in "Maze Name" field
2. Set width and height (5-30 cells)
3. Click **Generate** or edit by modifying the JSON files directly
4. Use **Sauver** to save your maze

## Algorithms

### A* (A-Star)
- Classic pathfinding algorithm
- Uses Manhattan distance heuristic
- Guarantees optimal path
- Visualizes explored nodes

### Q-Learning
- Reinforcement learning algorithm
- Learns optimal policy through trial and error
- Q-table based approach
- Configurable learning rate and discount factor

### Deep Q-Learning
- Neural network based Q-learning
- Handles larger state spaces
- Experience replay for stable learning
- Multiple hidden layers

### Evolutionary A*
- Genetic algorithm approach
- Evolves pathfinding strategies
- Adapts to maze characteristics
- Population-based optimization

### Meta-Learning
- Learns to select best algorithm
- Adapts to different maze types
- Improves with experience

## Building from Source (Command Line)

### Using CMake
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Debug
```

### Using MSBuild
```bash
msbuild SFML3-Robot.sln /p:Configuration=Debug /p:Platform=Win32
```

## Troubleshooting

### Common Issues and Solutions

#### "Cannot open include file: 'SFML/Graphics.hpp'"
- Verify SFML include path in project properties
- Ensure SFML is installed at `C:\SFML-2.5.1`

#### Linker Errors (LNK2019, LNK2005)
- Clean solution: **Build → Clean Solution**
- Rebuild: **Build → Rebuild Solution**
- Check for duplicate file inclusions

#### "Missing DLL" errors when running
- Copy all SFML DLLs to the executable directory
- Required DLLs: `sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`, `sfml-audio-2.dll`, `openal32.dll`

#### Application crashes on start
- Verify `config.txt` exists in working directory
- Check that font files are accessible
- Ensure asset paths are correct

#### No sound
- Verify `openal32.dll` is present
- Check that sound files exist in `assets/sounds/`

### Debug Build vs Release Build
- **Debug**: Uses `*-d.lib` libraries, more verbose logging
- **Release**: Optimized, uses standard libraries

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Coding Standards
- Use C++20 features where appropriate
- Follow existing naming conventions
- Add comments for complex logic
- Update documentation for new features

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **SFML Team** for the excellent multimedia library
- **OpenAI** for reinforcement learning concepts
- **Contributors** who helped improve the project
- **Stack Overflow** community for troubleshooting help

## Contact

- **Project Maintainer**: Ali ACHENAN
- **Email**: ali.jallousy12@gmail.com
- **GitHub**: [@AliAch04](https://github.com/AliAch04)
- **Project Link**: [https://github.com/AliAch04/SFML-RoboUIT](https://github.com/AliAch04/SFML-RoboUIT)

