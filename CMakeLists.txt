cmake_minimum_required(VERSION 3.0)
project(avis)

# shader compile function
find_program(EXEC_GLSLANG glslangValidator)
function(compile_spirv_shader INPUT_FILE OUTPUT_FILE)
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${EXEC_GLSLANG} -V ${INPUT_FILE} -o ${OUTPUT_FILE}
        DEPENDS ${INPUT_FILE}
        COMMENT "Compile SPIR-V shader '${OUTPUT_FILE}'"
    )
endfunction()

# libraries
IF(WIN32)
	set(LIB_VULKAN vulkan-1)
ELSE(WIN32)
	set(LIB_VULKAN vulkan)
ENDIF(WIN32)

# base program
include_directories(src include)

aux_source_directory(src             SRC_MAIN)
aux_source_directory(src/avis        SRC_AVIS)
aux_source_directory(src/avis/glfw   SRC_AVIS_GLFW)
aux_source_directory(src/avis/vulkan SRC_AVIS_VULKAN)

aux_source_directory(include/avis        INC_AVIS)
aux_source_directory(include/avis/utils  INC_AVIS_UTILS)
aux_source_directory(include/avis/vulkan INC_AVIS_VULKAN)
aux_source_directory(include/avis/vulkan INC_AVIS_GLFW)

set(SRC_AVIS_ALL ${SRC_AVIS} ${SRC_AVIS_VULKAN} ${SRC_AVIS_GLFW})
set(INC_AVIS_ALL ${INC_AVIS} ${INC_AVIS_UTILS} ${INC_AVIS_VULKAN} ${INC_AVIS_GLFW})

add_executable(main ${SRC_MAIN} ${SRC_AVIS_ALL} ${INC_AVIS_ALL})
target_link_libraries(main glfw ${LIB_VULKAN})

# shaders
set(DIR_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources)
file(MAKE_DIRECTORY ${DIR_RESOURCES})
compile_spirv_shader("${CMAKE_CURRENT_SOURCE_DIR}/resources/triangle.vert" "${DIR_RESOURCES}/triangle.vert.spv")
compile_spirv_shader("${CMAKE_CURRENT_SOURCE_DIR}/resources/triangle.frag" "${DIR_RESOURCES}/triangle.frag.spv")
add_custom_target(shaders DEPENDS "${DIR_RESOURCES}/triangle.vert.spv" "${DIR_RESOURCES}/triangle.frag.spv")
add_dependencies(main shaders)
