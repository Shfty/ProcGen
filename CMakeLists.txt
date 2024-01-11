cmake_minimum_required(VERSION 3.28)
project(ProcGen LANGUAGES CXX)

add_executable(
    Main

    main.cpp

    src/WorleyNoise.cpp
)

target_include_directories(
    Main

    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_package(PkgConfig REQUIRED)

pkg_search_module(GLM REQUIRED glm)

include_directories(${GLM_INCLUDE_DIRS})

target_link_libraries(Main ${GLM_LIBRARIES})
target_link_libraries(Main ${GLM_STATIC_LIBRARIES})
