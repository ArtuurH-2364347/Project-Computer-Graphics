cmake_minimum_required(VERSION 3.10)
project(F1_GraphicsProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(F1_Project
    glad.c
    src/main.cpp
    src/stb_imageDefinition.cpp
)

target_link_directories(F1_Project PRIVATE ${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(F1_Project PRIVATE glfw3 opengl32 assimp-vc143-mt)

add_custom_command(TARGET F1_Project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/bin/assimp-vc143-mt.dll"
    $<TARGET_FILE_DIR:F1_Project>
)