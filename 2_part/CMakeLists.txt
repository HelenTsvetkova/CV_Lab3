cmake_minimum_required(VERSION 3.5)

project(lab3_opencv_part2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# find_package( OpenCV 2.4.13.6 EXACT REQUIRED PATHS /home/helen/openCV2/ NO_DEFAULT_PATH)
FIND_PACKAGE( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )
link_directories( ${OpenCV_LIBRARY_DIR} )

set(SRC_LIST 
	main.cpp 
)

add_executable(${PROJECT_NAME} 
	${SRC_LIST}
)
TARGET_LINK_LIBRARIES (${PROJECT_NAME} 
	${OpenCV_LIBRARIES} 
)

