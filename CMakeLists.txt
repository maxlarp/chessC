cmake_minimum_required(VERSION 3.22)
project(chess LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_executable(chess main.c)

target_link_libraries(chess PRIVATE
  SDL3::SDL3
  SDL3_image::SDL3_image
)

target_compile_options(chess PRIVATE
  $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wshadow>
)

add_custom_command(
  TARGET chess POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:chess>/assets
  COMMENT "Copying assets to build folder..."
)
