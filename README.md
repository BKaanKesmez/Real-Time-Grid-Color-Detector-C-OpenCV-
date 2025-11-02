# Real-Time Grid Color Detector (C++ / OpenCV)

This is a C++/OpenCV application that performs real-time detection of a 9x9 grid on a piece of paper using a live video stream (e.g., an IP camera).

The application applies a bird's-eye view correction to the detected grid and identifies the dominant color within each individual cell (Red, Green, Blue, etc.). It uses a temporal stability check to filter out noise from hand movements or shadows and saves the final state of the grid to a text file upon exit.

[Insert a GIF or screenshot of the project in action here. An image showing the "Camera Feed" and "Grid Analysis" windows side-by-side is ideal.]

## 🚀 Project Definition and Purpose

The primary goal of this project is to create an interactive system that bridges a physical grid with a digital environment. Even as the camera moves, the system automatically finds the grid, corrects its perspective, and analyzes the colors within its cells.

## ✨ Key Features

This project successfully meets all of the following requirements:

* **Real-Time Bird's-Eye View:** Instantly flattens (warps) the skewed grid perspective from a live video feed.
* **Automatic Grid Detection:** Automatically detects the predefined 9x9 grid structure on the paper and highlights it with a green bounding box.
* **Individual Cell Identification:** Identifies and isolates all 81 individual cells within the flattened grid for analysis.
* **Dominant Color Detection:** Detects the dominant color in each cell (based on HSV color space), identifying colors like Red, Green, Blue, Yellow, White, Black, etc.
* **Visual Feedback Overlay:** Overlays the detected color name (e.g., "RED") onto the corresponding cell in the "Grid Analysis" output window.
* **Stability Filtering:** Filters out transient changes (like a hand waving or a brief shadow) and only registers persistent color changes.
* **Save to File:** Saves the final detected color layout of the entire grid to a `.txt` file when the program quits.

## 🛠️ Technology Stack

* **C++** (C++11 or higher)
* **OpenCV 4.x** (Core image processing library)
* **CMake** (Cross-platform build system)

---

## 🔧 Setup and Compilation (Windows)

Compiling this project requires a properly configured C++/OpenCV environment. The easiest and recommended method is to use the **vcpkg** package manager.

### Prerequisites

* **Visual Studio 2019 or 2022** (with "Desktop development with C++" and "C++ CMake tools for Windows" workloads installed)
* **vcpkg** (The C++ Library Manager)
* **Git**

### 1. Install vcpkg and OpenCV

If you don't have `vcpkg` installed, open a terminal (PowerShell) and run the following commands:

```bash
# Navigate to a directory where you want to install vcpkg (e.g., C:\dev)
cd C:\dev
git clone [https://github.com/Microsoft/vcpkg.git](https://github.com/Microsoft/vcpkg.git)
.\vcpkg\bootstrap-vcpkg.bat

# Install OpenCV with FFMPEG support (critical for video streaming)
# This step may take a significant amount of time
.\vcpkg\vcpkg.exe install opencv4[ffmpeg]:x64-windows

# Integrate vcpkg with Visual Studio (one-time command)
.\vcpkg\vcpkg.exe integrate install

```

###2. Compile the Project
Clone this GitHub repository:

Bash

git clone [URL_OF_THIS_REPOSITORY]
cd [PROJECT_FOLDER_NAME]
Open the project in Visual Studio 2022:

Launch Visual Studio.

On the start screen, select "Open a local folder" and choose the project folder you just cloned.

Visual Studio will automatically detect the CMakeLists.txt file, find the OpenCV libraries (thanks to vcpkg), and configure the project.

Press the green "Run" (▶️) button (or "Local Machine") in the top toolbar to build and run the project.


🖥️ How to Use
1. Physical Setup
Grid: Draw a 9x9 grid on a standard (e.g., A4) piece of paper. The grid should cover as much of the paper as possible (close to the edges).

Contrast: This is critical for automatic detection. Place the white paper on a dark, non-reflective surface (e.g., a dark green, black, or dark wood desk).

Lighting: Ensure good, diffuse lighting to prevent strong shadows from your hand or the camera.

2. Software Configuration
IP Camera: Install and run an IP camera app on your phone (e.g., "DroidCam" or "IP Webcam").

Video Source: Open the main.cpp file and update the VIDEO_SOURCE variable with the IP address provided by your phone's app:

C++

// ...
std::string video_source = "[http://192.168.1.128:4747/video](http://192.168.1.128:4747/video)"; // Set your camera's URL here
// ...
Run: Build and run the project from Visual Studio.

Controls
C++ Kamera Akisi (Algilama): Shows the raw camera feed with the green detection border overlaid.

C++ Grid Analizi (Duzlestirilmis): Shows the flattened, bird's-eye view of the grid with the detected color names written on each cell.

C++ Debug - Threshold: A black-and-white debug window showing what the detection algorithm "sees" to find the paper.

q or ESC: While focused on one of the app's windows, press 'q' or 'ESC' to quit the program and save the final grid state.

📄 Output
When the program is closed (via 'q' or 'ESC'), it saves a file named grid_sonuc.txt to the application's working directory (e.g., in your build/x64-Debug/ or out/build/x64-Debug/ folder).

This file contains a neatly formatted table of the last known stable color for all 81 cells.
