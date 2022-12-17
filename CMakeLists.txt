cmake_minimum_required(VERSION 3.5)

project(sjtu_proj VERSION 0.0.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "[rxy] generator: ${CMAKE_GENERATOR}")

# set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" FORCE)

set(CONF_PATH ${CMAKE_CURRENT_LIST_DIR}/conf.json)

set(RSRP_TYPE "double")
set(POINT_TYPE "double")
set(PROB_TYPE ${POINT_TYPE})
set(ND_MU 0.0)
set(ND_SIGMA 3.0)
set(EPSILON 1e-6)
set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# add_compile_options(-Wall -Wextra -Werror=return-type)

add_definitions(-DDEBUG)
# if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # if (MSVC) 
    #     add_compile_options(/fsanitize=address)
    # else()
    #     add_compile_options(-fsanitize=address -Wno-unused-parameter -Wno-unused-function)
    #     add_link_options(-fsanitize=address)
    # endif()
# endif()

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

configure_file(configure.hpp.in configure.hpp)
include_directories(${CMAKE_BINARY_DIR})

find_package(Boost REQUIRED COMPONENTS json)
find_path(MLPACK_INCLUDE_DIRS "mlpack/mlpack.hpp" REQUIRED)
find_package(Armadillo CONFIG REQUIRED)

add_subdirectory(lib/parser1)
add_subdirectory(lib/hmm)
add_subdirectory(lib/freq)
list(APPEND LIBS parser1 hmm freq)

add_subdirectory(lib/OpenXLSX)
list(APPEND LIBS OpenXLSX::OpenXLSX)

file(GLOB_RECURSE SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} 
PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${Boost_INCLUDE_DIRS} 
    ${MLPACK_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} 
PRIVATE 
    ${LIBS} 
    ${Boost_LIBRARIES} 
    ${ARMADILLO_LIBRARIES}
)
