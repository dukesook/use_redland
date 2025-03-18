cmake_minimum_required(VERSION 3.10)

project(use_redland)

find_package(PkgConfig REQUIRED)
pkg_check_modules(REDLAND REQUIRED redland)

add_executable(use_redland main.cpp)

target_include_directories(use_redland PRIVATE ${REDLAND_INCLUDE_DIRS})
target_link_libraries(use_redland PRIVATE ${REDLAND_LIBRARIES})