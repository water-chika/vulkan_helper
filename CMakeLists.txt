cmake_minimum_required(VERSION 3.20)

project(vulkan_helper)

find_package(Vulkan REQUIRED)

add_library(vulkan_helper
    vulkan_helper.cpp
    vulkan_helper.hpp
    spirv_helper.hpp)
target_include_directories(vulkan_helper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vulkan_helper PUBLIC Vulkan::Vulkan)
set_target_properties(vulkan_helper PROPERTIES CXX_STANDARD 23)

if(WIN32)

add_executable(vulkan_helper_test
    main.cpp
    comp.spv)

set_target_properties(vulkan_helper_test PROPERTIES CXX_STANDARD 23)
target_link_libraries(vulkan_helper_test PUBLIC vulkan_helper)

endif()

add_custom_command(OUTPUT comp.spv
  COMMAND Vulkan::glslangValidator --target-env vulkan1.3
              ${CMAKE_CURRENT_SOURCE_DIR}/test.comp
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/test.comp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.comp Vulkan::glslangValidator)
