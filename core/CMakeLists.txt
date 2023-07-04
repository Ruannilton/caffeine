cmake_minimum_required(VERSION 3.5)
project(CoreLib C)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)

# Set the output directory for the library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set the include directories
include_directories(include)

# Add the source files
file(GLOB SOURCES src/*.c)

# Create the static library
add_library(CoreLib STATIC ${SOURCES})

# Set the public headers
set(PUBLIC_HEADERS 
    include/caffeine_algorithms.h
    include/caffeine_core.h
    include/caffeine_flags.h
    include/caffeine_macros.h
    include/caffeine_memory.h
    include/caffeine_string.h
)



# Install the library
install(TARGETS CoreLib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
