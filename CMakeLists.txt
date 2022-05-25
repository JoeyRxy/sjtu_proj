cmake_minimum_required(VERSION 3.5)

project(sjtu_proj)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RSRP_TYPE "int")
set(POINT_TYPE "double")
set(PROB_TYPE ${POINT_TYPE})
set(MAP_SIZE 100)
set(ND_MU 0.0)
set(ND_SIGMA 1.0)
set(ECCENTRICITY 0.1)
set(MINKOWSKI_P 2)
set(ENUMER_LIMIT 1000000000)

configure_file(configure.hpp.in configure.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(BOOST_ROOT "/home/rxy/files/boost_1_78_0")

find_package(Boost REQUIRED COMPONENTS)

find_package(Eigen3 REQUIRED COMPONENTS)

find_package(GSL REQUIRED COMPONENTS)

add_subdirectory(lib/parser1)
list(APPEND LIBS parser1)
add_subdirectory(lib/hmm)
list(APPEND LIBS hmm)

file(GLOB_RECURSE SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS} Eigen3::Eigen GSL::gsl Boost::boost)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
