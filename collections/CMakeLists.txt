cmake_minimum_required(VERSION 3.5)
project(CollectionsLib C)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)

# Set the output directory for the library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set the include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../core)

# Add the source files
file(GLOB SOURCES *.c)

# Create the static library
add_library(CollectionsLib STATIC ${SOURCES})

# Set the public headers
set(PUBLIC_HEADERS 
    caffeine_array.h
    caffeine_collections.h
    caffeine_container_type.h
    caffeine_container_interface.h
    caffeine_iterator.h
    caffeine_ord_container_type.h
    caffeine_ordered_vector.h
    caffeine_vector.h
    caffeine_hashmap.h
    caffeine_linked_list.h
)

set(PRIVATE_HEADERS
    include_private/caffeine_container_functions.h
    include_private/caffeine_ord_container_functions.h
    # Add more private headers if needed
)

# Add the dependency on CoreLib
target_link_libraries(CollectionsLib CoreLib)

# Install the library
install(TARGETS CollectionsLib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    PRIVATE_HEADER DESTINATION private
)
