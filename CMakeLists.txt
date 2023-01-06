cmake_minimum_required(VERSION 3.0.0)
project(gbemu_v2 VERSION 0.1.0)
file(GLOB SOURCES "src/*.cpp")
file(GLOB INCLUDES "include/*.hpp")

# include(CTest)
# enable_testing()

include_directories(include)
add_executable(gbemu_v2 ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
