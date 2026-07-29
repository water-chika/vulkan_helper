cmake_minimum_required(VERSION 3.20)

project(vulkan_helper)

if (NOT TARGET Vulkan::Vulkan)
if (TARGET vulkan)
add_library(Vulkan::Vulkan ALIAS vulkan)
else()
find_package(Vulkan REQUIRED)
endif()
endif()

add_library(vulkan_helper
    vulkan_helper.cpp
    vulkan_helper.hpp
    spirv_helper.hpp
    platform.hpp)
target_include_directories(vulkan_helper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vulkan_helper PUBLIC
    Vulkan::Vulkan
    cpp_helper
)
set_target_properties(vulkan_helper PROPERTIES CXX_STANDARD 23)

add_custom_command(OUTPUT comp.spv
  COMMAND glslang-standalone --target-env vulkan1.3
              ${CMAKE_CURRENT_SOURCE_DIR}/test.comp
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/test.comp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.comp
    glslang-standalone)
