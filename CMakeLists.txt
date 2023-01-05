set(EXEC "gbemu")
file(GLOB INCLUDES CMAKE_CONFIGURE_DEPENDS "src/include/*.hpp")
file(GLOB SOURCES CMAKE_CONFIGURE_DEPENDS "src/*.cpp")

cmake_minimum_required(VERSION 3.10)

project(${EXEC})

add_executable(${EXEC} ${SOURCES} ${INCLUDES})
target_include_directories(${EXEC} PRIVATE ${INCLUDES})