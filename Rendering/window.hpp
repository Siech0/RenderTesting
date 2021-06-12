#pragma once
  
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <string_view>
#include <memory>
#include <utility>
#include <tuple>
#include <functional>

namespace sys
{
	class Window
	{
		// Functor used for destroying windows
		struct DestroyWindow {
			void operator()(GLFWwindow* ptr)
			{
				glfwDestroyWindow(ptr);
			}
		};

		using window_handle = std::unique_ptr<GLFWwindow, DestroyWindow>;

		std::string m_title;
		window_handle m_windowHandle;


	public:
		Window(int width, int height, const std::string& name);
		~Window();

		Window(const Window&) = delete;
		Window(Window&&) = default;

		// Statics
		static void pollEvents();
		static void initializeSystem();
		static void terminateSystem();

		// Status
		bool shouldClose() const;


		// Setters

		// Getters
		std::pair<int, int> getSize() const;
		std::tuple<int, int, int, int> getFrameSize() const;
		std::pair<int, int> getPosition() const;
		std::string getTitle() const;


		// Other
		vk::UniqueSurfaceKHR createSurface(vk::Instance instance) const;
	};
}