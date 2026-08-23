cmake_minimum_required(VERSION 3.20)

project(Vk-Raytracer)

if(NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    message(WARNING "Vulkan SDK not detected")
else()
    message(STATUS "Found Vulkan SDK at $ENV{VULKAN_SDK}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(external/sdl EXCLUDE_FROM_ALL)
add_subdirectory(external/vma EXCLUDE_FROM_ALL)

# FETCH GLM
include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp
    src/Application.cpp
    src/Core.cpp
    src/Renderer.cpp
    src/Queue.cpp
    src/Swapchain.cpp
    src/ComputePipeline.cpp
    src/ComputeDescriptorSet.cpp
    src/Vma.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        SDL3::SDL3
        glm::glm
        GPUOpen::VulkanMemoryAllocator
)

# Other project specific header files
# target_include_directories()

if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-nullability-extension
    )
endif()
