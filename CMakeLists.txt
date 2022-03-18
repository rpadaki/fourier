cmake_minimum_required(VERSION 3.21)

project(fourier)

# Compile and link

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_library(AVCODEC_LIBRARY avcodec)
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
include_directories(${AVCODEC_INCLUDE_DIR})

add_executable(fourier src/main.c)
target_link_libraries(fourier ${SDL2_LIBRARIES} ${AVCODEC_LIBRARY})

find_program(CLANG_FORMAT NAMES "clang-format")
if (NOT CLANG_FORMAT)
    message(STATUS "clang-format not found")   
endif()

# Format

file(GLOB_RECURSE SOURCE_FILES src/*.c)
file(GLOB_RECURSE HEADER_FILES src/*.h)

add_custom_target(
    format
    COMMAND ${CLANG_FORMAT} -style=file -i ${SOURCE_FILES} ${HEADER_FILES}
    DEPENDS ${SOURCE_FILES} ${HEADER_FILES}
)

# Run
add_custom_target(
    run
    ${CMAKE_BINARY_DIR}/fourier
    DEPENDS fourier
)
