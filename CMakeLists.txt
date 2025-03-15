cmake_minimum_required(VERSION 3.10)
project(ML_RTOS C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add include directory
include_directories(include)

# Define source files
set(SOURCES
    src/main.c
    src/core/kernel.c
    src/core/scheduler.c
    src/core/task_manager.c
    src/core/memory_matrix.c
    src/ml/ml_engine.c
    src/fault/fault_tolerance.c
)

# Create executable
add_executable(ml_rtos ${SOURCES})

# For Windows specific libraries
if(WIN32)
    target_link_libraries(ml_rtos winmm)
endif()

# Set warnings
if(MSVC)
    target_compile_options(ml_rtos PRIVATE /W4)
else()
    target_compile_options(ml_rtos PRIVATE -Wall -Wextra)
endif()

# Output binary to bin directory
set_target_properties(ml_rtos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Installation instructions
install(TARGETS ml_rtos DESTINATION bin)

# Print configuration information
message(STATUS "Configured ML RTOS project with:")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
