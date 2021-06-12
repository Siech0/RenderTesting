#pragma once

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <unordered_set>

#include "window.hpp"

namespace gfx
{
	class VulkanApp
	{
		// Window
		std::shared_ptr<sys::Window> m_window;

		// Vulkan handles
		vk::UniqueInstance m_instance;
		vk::UniqueSurfaceKHR m_surface;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;

		// Bookkeeping
		std::unordered_set<uint32_t> m_uniqueQueueFamilyIndices;

#ifndef NDEBUG
		vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> m_debugMessenger;
#endif

	public:
		VulkanApp(std::shared_ptr<sys::Window> window);
		~VulkanApp();
	private:

		static VKAPI_ATTR VkBool32 VulkanApp::debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		void createInstance();
		void createSurface();
		void createDevice();
		void createSwapchain();
	};
}

