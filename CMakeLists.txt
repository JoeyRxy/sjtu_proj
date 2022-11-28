cmake_minimum_required(VERSION 3.5)

project(sjtu_proj)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXPORT_COMPILE_COMMANDS ON)

message(STATUS "[rxy] generator: ${CMAKE_GENERATOR}")

set(CONF_PATH ${CMAKE_CURRENT_LIST_DIR}/conf.json)

set(RSRP_TYPE "double")
set(POINT_TYPE "double")
set(PROB_TYPE ${POINT_TYPE})
set(ND_MU 0.0)
set(ND_SIGMA 3.0)
set(EPSILON 1e-6)

# add_compile_options(-Wall -Wextra -Werror=return-type)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_definitions(-DDEBUG)
    # add_compile_options(-fsanitize=address -Wno-unused-parameter -Wno-unused-function)
    # add_link_options(-fsanitize=address)
endif()


configure_file(configure.hpp.in configure.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED COMPONENTS)
link_libraries(Boost::boost)

# find_package(Eigen3 REQUIRED COMPONENTS)

# find_package(GSL REQUIRED COMPONENTS)

add_subdirectory(lib/parser1)
list(APPEND LIBS parser1)
add_subdirectory(lib/hmm)
list(APPEND LIBS hmm)
add_subdirectory(lib/freq)
list(APPEND LIBS freq)

add_subdirectory(lib/OpenXLSX)
list(APPEND LIBS OpenXLSX::OpenXLSX)

file(GLOB_RECURSE SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost ${LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
