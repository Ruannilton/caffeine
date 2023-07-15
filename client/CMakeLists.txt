cmake_minimum_required(VERSION 3.5)
project(YourAppName)

add_subdirectory(../engine)

target_link_libraries(YourAppName PRIVATE CaffeineLib)

target_include_directories(YourAppName PRIVATE ../engine)