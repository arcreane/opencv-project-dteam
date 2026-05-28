cmake_minimum_required(VERSION 3.16)
project(MyImageEditor VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV (version 4.x recommended)
find_package(OpenCV 4 REQUIRED)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Auto-MOC for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ImageProcessor.cpp
    src/HistogramWidget.cpp
    src/UndoManager.cpp
    src/PanoramaProcessor.cpp
)

set(HEADERS
    include/MainWindow.h
    include/ImageProcessor.h
    include/HistogramWidget.h
    include/UndoManager.h
    include/PanoramaProcessor.h
)

add_executable(MyImageEditor ${SOURCES} ${HEADERS})

target_include_directories(MyImageEditor PRIVATE include)

target_link_libraries(MyImageEditor
    ${OpenCV_LIBS}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Copy sample images to build directory
file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/resources)
