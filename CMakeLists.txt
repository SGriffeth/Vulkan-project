cmake_minimum_required(VERSION 3.19.0)

project(MINEANARCHY VERSION 1.0)

set(VERTEX_SHADER "${PROJECT_SOURCE_DIR}/shaders/vert.glsl")
set(FRAGMENT_SHADER "${PROJECT_SOURCE_DIR}/shaders/frag.glsl")

configure_file(${PROJECT_SOURCE_DIR}/config/${PROJECT_NAME}Config.h.in ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}Config.h)

add_executable(${PROJECT_NAME} main.cpp descriptorManager.cpp skeletalAnimation.cpp)

add_subdirectory(external/glm)
add_subdirectory(external/assimp)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD}")
        endif()
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

execute_process(COMMAND glslc -fshader-stage=vert ${VERTEX_SHADER} -o ${VERTEX_SHADER}.spv)
execute_process(COMMAND glslc -fshader-stage=frag ${FRAGMENT_SHADER} -o ${FRAGMENT_SHADER}.spv)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

target_include_directories(${PROJECT_NAME} PRIVATE external/glm)
target_include_directories(${PROJECT_NAME} PRIVATE Vulkan)
