cmake_minimum_required(VERSION 3.10)
project(sq)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

add_executable(shaders main.cpp application.cpp gl_arcball.cpp gl_viewer.cpp obj.cpp)
target_link_libraries(shaders ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY})
