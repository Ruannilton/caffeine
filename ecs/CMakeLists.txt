cmake_minimum_required(VERSION 3.5)
project(ECSLib C)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)

# Set the output directory for the library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set the include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../collections/include
    )

# Add the source files
file(GLOB SOURCES src/*.c)

# Create the static library
add_library(ECSLib STATIC ${SOURCES})

# Set the public headers
set(PUBLIC_HEADERS 
    include/ecs.h
)



# Install the library
install(TARGETS ECSLib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
