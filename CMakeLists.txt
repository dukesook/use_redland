cmake_minimum_required(VERSION 3.10)

# Set the project name
project(use_redland)

# Find the Redland library
find_package(PkgConfig REQUIRED)
pkg_check_modules(REDLAND REQUIRED redland)

# Add the executable
add_executable(use_redland main.cpp)

# Link Redland
target_include_directories(use_redland PRIVATE ${REDLAND_INCLUDE_DIRS})
target_link_libraries(use_redland PRIVATE ${REDLAND_LIBRARIES})