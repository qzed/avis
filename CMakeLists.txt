cmake_minimum_required(VERSION 3.0)
project(avis)

# TODO: proper find-scripts

# glslangValidator
find_program(EXEC_GLSLANG glslangValidator)

# vulkan
find_path(VULKAN_INCLUDE_DIR
    NAMES
        "vulkan/vulkan.h"
    HINTS
        "${VULKAN_LOCATION}/include"
        "$ENV{VULKAN_LOCATION}/include"
    PATHS
        "/usr/include/"
    DOC
        "vulkan include directory"
)

IF(WIN32)
	find_library(VULKAN_LIBRARY NAMES vulkan-1)
ELSE(WIN32)
	find_library(VULKAN_LIBRARY NAMES vulkan)
ENDIF(WIN32)

# GLFW
find_library(GLFW_LIBRARY NAMES glfw glfw3)

find_path(GLFW_INCLUDE_DIR
    NAMES
        "GLFW/glfw3.h"
    HINTS
        "${GLFW_LOCATION}/include"
        "$ENV{GLFW_LOCATION}/include"
    PATHS
        "/usr/include/"
    DOC
        "glfw include directory"
)


# shader compile function
function(compile_spirv_shader INPUT_FILE OUTPUT_FILE)
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${EXEC_GLSLANG} -V ${INPUT_FILE} -o ${OUTPUT_FILE}
        DEPENDS ${INPUT_FILE}
        COMMENT "Compile SPIR-V shader '${OUTPUT_FILE}'"
    )
endfunction()


# base sources
include_directories(src include ${VULKAN_INCLUDE_DIR} ${GLFW_INCLUDE_DIR})

aux_source_directory(src/avis            SRC_AVIS)
aux_source_directory(src/avis/glfw       SRC_AVIS_GLFW)
aux_source_directory(src/avis/vulkan     SRC_AVIS_VULKAN)

aux_source_directory(include/avis        INC_AVIS)
aux_source_directory(include/avis/utils  INC_AVIS_UTILS)
aux_source_directory(include/avis/vulkan INC_AVIS_VULKAN)
aux_source_directory(include/avis/vulkan INC_AVIS_GLFW)

set(SRC_AVIS_ALL ${SRC_AVIS} ${SRC_AVIS_VULKAN} ${SRC_AVIS_GLFW})
set(INC_AVIS_ALL ${INC_AVIS} ${INC_AVIS_UTILS} ${INC_AVIS_VULKAN} ${INC_AVIS_GLFW})


# main app
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resources")

compile_spirv_shader("${CMAKE_CURRENT_SOURCE_DIR}/resources/avis/ringbuffer.vert" "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.vert.spv")
compile_spirv_shader("${CMAKE_CURRENT_SOURCE_DIR}/resources/avis/ringbuffer.frag" "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.frag.spv")

add_custom_target(shaders DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.frag.spv"
)

add_executable(main ${SRC_AVIS_ALL} ${INC_AVIS_ALL})
target_link_libraries(main ${GLFW_LIBRARY} ${VULKAN_LIBRARY})

add_dependencies(main shaders)
