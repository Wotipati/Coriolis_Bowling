cmake_minimum_required(VERSION 3.9)
project(coriolisBowling)

set(TARGET_DIR "${CMAKE_CURRENT_LIST_DIR}")

if (EXISTS "${TARGET_DIR}/common.h.in")
    configure_file("${TARGET_DIR}/common.h.in"
            "${TARGET_DIR}/common.h" @ONLY)
endif()

# ------------------------------------------------------------------------------
# Find OpenGL libraries
# ------------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)



## ------------------------------------------------------------------------------
## Set output path
## ------------------------------------------------------------------------------
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})


set(CMAKE_CXX_STANDARD 11)

add_executable(coriolisBowling
        common.h
        main.cpp
        stb_image.h
        tiny_obj_loader.h
)


# ------------------------------------------------------------------------------
# Setup dependencies
# ------------------------------------------------------------------------------
set(ALL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS}
        ${GLFW3_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR})

set(ALL_LIBRARIES ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY} ${GLEW_LIBRARY})

if (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW3 REQUIRED glfw3)
    set(ALL_LIBRARIES ${ALL_LIBRARIES} ${CMAKE_DL_LIBS} ${GLFW3_STATIC_LIBRARIES})
endif()

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework IOKit -framework CoreVideo")
endif()

target_link_libraries(coriolisBowling glew glfw3 ${ALL_LIBRARIES} ${CMAKE_EXE_LINKER_FLAGS} )
