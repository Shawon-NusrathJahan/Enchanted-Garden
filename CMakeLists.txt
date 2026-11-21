
cmake_minimum_required(VERSION 3.20)

project(EnchantedGarden)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(EnchantedGarden
    src/main.cpp
    src/mountain.cpp
    src/village.cpp
    src/garden.cpp
    src/animation.cpp
)

target_include_directories(EnchantedGarden PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(EnchantedGarden PRIVATE
    glfw
    GLEW::GLEW
    glm::glm
    OpenGL::GL
)
