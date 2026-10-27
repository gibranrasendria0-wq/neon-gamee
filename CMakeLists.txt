cmake_minimum_required(VERSION 3.16)
project(NeonSurvivor LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(neon-survivor src/main.cpp)
target_link_libraries(neon-survivor PRIVATE raylib)

if(EMSCRIPTEN)
  set_target_properties(neon-survivor PROPERTIES SUFFIX ".html")
  target_link_options(neon-survivor PRIVATE
    "-sUSE_GLFW=3"
    "-sASYNCIFY"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sASSERTIONS=1"
    "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
  )
  target_link_libraries(neon-survivor PRIVATE "-sUSE_GLFW=3")
  set_target_properties(neon-survivor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist"
  )
endif()
