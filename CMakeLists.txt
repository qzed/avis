cmake_minimum_required(VERSION 3.0)
project(avis)

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
target_link_libraries(main glfw vulkan)
