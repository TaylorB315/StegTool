cmake_minimum_required(VERSION 3.0)
project(MyProject)

# Locate OpenCV
find_package(OpenCV REQUIRED)

# Add include directories for OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Add an include directory
include_directories(headers)

# Add your executable
add_executable(myProgram src/main.cpp src/Image.cpp)

# Link against OpenCV libraries
target_link_libraries(myProgram ${OpenCV_LIBS})
