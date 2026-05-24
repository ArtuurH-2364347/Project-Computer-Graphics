cmake_minimum_required(VERSION 3.10)
project(F1_GraphicsProject)

set(CMAKE_CXX_STANDARD 17)

#set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
#add_subdirectory(external/assimp)

#find_package(assimp REQUIRED)

# 1. Zoek GLFW (deze heeft wel een config file)
#set(GLFW_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
# add_subdirectory(external/glfw)
#find_package(glfw REQUIRED)

include_directories(/include)

# 3. Zorg dat je eigen include map (voor glad) ook gevonden wordt
include_directories(${PROJECT_SOURCE_DIR}/include)

# 4. De executable bouwen
add_executable(F1_Project src/main.cpp glad.c src/stb_imageDefinition.cpp)

# 5. Linken aan GLFW en de standaard OpenGL library van Linux
# target_link_libraries(F1_Project glfw GL assimp) # => linux
target_link_libraries(F1_Project glfw opengl32 assimp) # => linux