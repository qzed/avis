cmake_minimum_required(VERSION 3.0)
project(avis)


set (CMAKE_CXX_STANDARD 14)

#dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(Vulkan    REQUIRED)
find_package(GLFW      REQUIRED)
find_package(PortAudio REQUIRED)
find_package(FFMPEG    REQUIRED)
find_package(Boost     REQUIRED)
find_program(EXEC_GLSLANG glslangValidator)


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
include_directories(
    src include
    ${VULKAN_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

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

compile_spirv_shader(
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/avis/ringbuffer.vert"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.vert.spv"
)

compile_spirv_shader(
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/avis/ringbuffer.frag"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.frag.spv"
)

add_custom_target(shaders DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/ringbuffer.frag.spv"
)

add_executable(avis ${SRC_AVIS_ALL} ${INC_AVIS_ALL})
target_link_libraries(avis ${GLFW_LIBRARIES} ${VULKAN_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES})
add_dependencies(avis shaders)
