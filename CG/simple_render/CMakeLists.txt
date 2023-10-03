cmake_minimum_required(VERSION 3.12)

project(simple_render)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(${PROJECT_NAME} sample.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT m GLEW::GLEW glm::glm)

