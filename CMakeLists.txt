cmake_minimum_required(VERSION 3.5)
project(CaffeineLib C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(./core CoreLib)
add_subdirectory(./collections CollectionsLib)
add_subdirectory(./ecs ECSLib)


# Create an empty source file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/empty.c "")

# Create the CaffeineLib target with the empty source file
add_library(CaffeineLib STATIC ${CMAKE_CURRENT_BINARY_DIR}/empty.c)

# Link against CoreLib and CollectionsLib
target_link_libraries(CaffeineLib PUBLIC
    CoreLib
    CollectionsLib
    ECSLib
)

# Install the library
install(TARGETS CaffeineLib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)