cmake_minimum_required(VERSION 3.10)
project(GatorDB)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")  # Include debug symbols, no optimization
set(CMAKE_BUILD_TYPE Debug)

# 1. First, find the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 2. Second, define the executable
#add_executable(gator_db "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" ${SOURCES})
add_executable(gator_db
    main.cpp
    src/executer.cpp
    src/storage/disk_manager.cpp
    src/storage/pager.cpp
    src/catalog/catalog.cpp
    src/storage/table_page.cpp
    src/storage/tuple.cpp    
    src/storage/schema.cpp   
    src/storage/storage_utils.cpp
    src/common/statement.cpp
)

# 3. Third, apply settings TO that executable
target_include_directories(gator_db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# 4. Finally, link libraries
find_package(Threads REQUIRED)
target_link_libraries(gator_db PUBLIC Threads::Threads)