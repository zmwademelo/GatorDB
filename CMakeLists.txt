cmake_minimum_required(VERSION 3.10)
project(GatorDB)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. First, find the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 2. Second, define the executable
# Since main.cpp is in the same folder as this CMakeLists.txt:
add_executable(gator_db "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" ${SOURCES})

# 3. Third, apply settings TO that executable
target_include_directories(gator_db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# 4. Finally, link libraries
find_package(Threads REQUIRED)
target_link_libraries(gator_db PUBLIC Threads::Threads)