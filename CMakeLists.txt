set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)

cmake_minimum_required(VERSION 3.16) # Abseil generally prefers 3.16+ for FetchContent
project(GatorDB)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- NEW: Abseil Setup ---
include(FetchContent)
FetchContent_Declare(
  absl
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20240722.0 # Stable LTS
)

# Force Abseil to use the standard library and avoid Intel-specific flags
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)
set(ABSL_PROPAGATE_CXX_STANDARD ON CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL ON CACHE BOOL "" FORCE)

# This prevents Abseil from overriding your C++17 setting
set(ABSL_PROPAGATE_CXX_STANDARD ON)
set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_MakeAvailable(absl)
# -------------------------

# 1. First, find the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 2. Second, define the executable
#add_executable(gator_db "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" ${SOURCES})
add_executable(gator_db
    main.cpp
    src/executer.cpp
    src/storage/disk_manager.cpp
    src/storage/pager.cpp
    src/storage/table_page.cpp
    src/storage/tuple.cpp    # <--- Is this here?
    src/storage/schema.cpp   # <--- Is this here?
    src/common/statement.cpp
)

# 3. Third, apply settings TO that executable
target_include_directories(gator_db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# 4. Finally, link libraries
find_package(Threads REQUIRED)
# We need to link these specific components for StatusOr to work on ARM64
target_link_libraries(gator_db PUBLIC 
    Threads::Threads 
    absl::status
    absl::statusor
    absl::strings      # Required by status
    absl::raw_logging_internal # Required for Abseil's internal error handling
)