# CMakeList.txt : CMake project for Rendering, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)     
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (Rendering "rendering.cpp" "vulkan_app.hpp" "vulkan_app.cpp" "window.hpp" "window.cpp")

target_include_directories(Rendering
	PRIVATE ${GLFW_INCLUDE_DIRS}
	PRIVATE ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Rendering
	PRIVATE spdlog::spdlog spdlog::spdlog_header_only
	PRIVATE nlohmann_json nlohmann_json::nlohmann_json
	PRIVATE	fmt::fmt-header-only
	PRIVATE glfw
	PRIVATE Vulkan::Vulkan
	PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator
)

set_property(TARGET Rendering PROPERTY CXX_STANDARD 17)

# TODO: Add tests and install targets if needed.
