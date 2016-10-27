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

# base setup
include_directories(src include)

aux_source_directory(src/avis            SRC_AVIS)
aux_source_directory(src/avis/glfw       SRC_AVIS_GLFW)
aux_source_directory(src/avis/vulkan     SRC_AVIS_VULKAN)

aux_source_directory(include/avis        INC_AVIS)
aux_source_directory(include/avis/utils  INC_AVIS_UTILS)
aux_source_directory(include/avis/vulkan INC_AVIS_VULKAN)
aux_source_directory(include/avis/vulkan INC_AVIS_GLFW)

set(SRC_AVIS_LIB ${SRC_AVIS} ${SRC_AVIS_VULKAN} ${SRC_AVIS_GLFW})
set(INC_AVIS_LIB ${INC_AVIS} ${INC_AVIS_UTILS} ${INC_AVIS_VULKAN} ${INC_AVIS_GLFW})

# triangle example
# TODO: move to subdirectory
set(DIR_EXAMPLES_TRIANGLE "${CMAKE_CURRENT_SOURCE_DIR}/examples/triangle")
aux_source_directory(${DIR_EXAMPLES_TRIANGLE} SRC_EXAMPLES_TRIANGLE)
add_executable(triangle ${SRC_EXAMPLES_TRIANGLE} ${SRC_AVIS_LIB} ${INC_AVIS_LIB})
target_link_libraries(triangle glfw ${LIB_VULKAN})

set(DIR_EXAMPLES_TRIANGLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/resources")
file(MAKE_DIRECTORY "${DIR_EXAMPLES_TRIANGLE_RESOURCES}")
compile_spirv_shader("${DIR_EXAMPLES_TRIANGLE}/triangle.vert" "${DIR_EXAMPLES_TRIANGLE_RESOURCES}/triangle.vert.spv")
compile_spirv_shader("${DIR_EXAMPLES_TRIANGLE}/triangle.frag" "${DIR_EXAMPLES_TRIANGLE_RESOURCES}/triangle.frag.spv")
add_custom_target(shaders DEPENDS "${DIR_EXAMPLES_TRIANGLE_RESOURCES}/triangle.vert.spv" "${DIR_EXAMPLES_TRIANGLE_RESOURCES}/triangle.frag.spv")
add_dependencies(triangle shaders)
