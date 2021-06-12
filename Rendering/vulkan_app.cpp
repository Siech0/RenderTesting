
#include <fmt/format.h>

#include <string>
#include <cstdint>
#include <algorithm>
#include <limits>

#include <vector>
#include <unordered_set>

#include "vulkan_app.hpp"

// https://github.com/dokipen3d/vulkanHppMinimalExample/blob/master/main.cpp Good reference


namespace gfx
{
	VulkanApp::VulkanApp(std::shared_ptr<sys::Window> window) :
		m_window(window)
	{	
		createInstance();
		createSurface();
		createDevice();
		createSwapchain();
	}

	VulkanApp::~VulkanApp()
	{

	}

	VKAPI_ATTR VkBool32 VulkanApp::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			spdlog::debug("VULKAN: {}", pCallbackData->pMessage);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			spdlog::info("VULKAN: {}", pCallbackData->pMessage);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			spdlog::warn("VULKAN: {}", pCallbackData->pMessage);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			spdlog::error("VULKAN: {}", pCallbackData->pMessage);
			break;
		}
		return VK_FALSE;
	}

	void VulkanApp::createInstance()
	{
		// General application info
		vk::ApplicationInfo app_info(
			"Renderer",					// App Name
			VK_MAKE_VERSION(1, 0, 0),	// App Version
			"No Engine", 				// App Engine
			VK_MAKE_VERSION(1, 0, 0),	// Engine Version
			VK_API_VERSION_1_2			// API Version
		);

		// Load instance extensions
		uint32_t glfw_extension_count = 0u;
		auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		
#ifndef NDEBUG
		// Enable debug extensions
		extensions.push_back("VK_EXT_debug_utils");
#endif

		// Load instance layers
		std::vector<const char*> layers;
#ifndef NDEBUG
		// Enable debug layers
		layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

		// Create the vulkan instance
		vk::InstanceCreateInfo instance_create_info(
			{},										// Flags
			&app_info,								// Application info
			static_cast<uint32_t>(layers.size()),	// Layer count
			layers.data(),							// Layers data
			static_cast<uint32_t>(extensions.size()),// Extensions count
			extensions.data()						// Extensions
		);

		m_instance = vk::createInstanceUnique(instance_create_info);


#ifndef NDEBUG
		// Create debug utils messenger
		auto dldi = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);

		vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info(
			{},			// Flags
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			VulkanApp::debugCallback
		);

		m_debugMessenger = m_instance->createDebugUtilsMessengerEXTUnique(
			debug_utils_create_info,	// Create info
			nullptr,
			dldi
		);
#endif

	}

	void VulkanApp::createSurface()
	{
		m_surface = m_window->createSurface(*m_instance);
	}

	void VulkanApp::createDevice()
	{
		auto physical_devices = m_instance->enumeratePhysicalDevices();

		spdlog::info("--- Physical Devices ---");
		std::vector<int> device_scores(physical_devices.size(), 0);
		for(size_t i = 0; i < physical_devices.size(); ++i)
		{
			auto& d = physical_devices[i];
			auto& score = device_scores[i];

			auto props = d.getProperties();
			spdlog::info("{}:", props.deviceName);
			spdlog::info("\tID: {}", props.deviceID);
			spdlog::info("\tType: {}", props.deviceType);
			spdlog::info("\tAPI Version: {}", props.apiVersion);
			spdlog::info("\tVendor ID: {}", props.vendorID);

			if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				score += 100000; // Heavily prefer discrete GPUS
			}

			auto mem_props = d.getMemoryProperties();
			spdlog::info("\tMemory:");
			for(size_t heap_num = 0; heap_num < mem_props.memoryHeapCount; ++heap_num)
			{
				auto& heap = mem_props.memoryHeaps[heap_num];
				score += heap.size;
				spdlog::info("\t\tHeap {}", heap_num);
				spdlog::info("\t\t\tSize: {}", heap.size);
				spdlog::info("\t\t\tFlags: DEVICE_LOCAL = {}, MULTI_INSTANCE = {}, MULTI_INSTANCE_KHR = {}", 
					static_cast<bool>(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal),
					static_cast<bool>(heap.flags & vk::MemoryHeapFlagBits::eMultiInstance),
					static_cast<bool>(heap.flags & vk::MemoryHeapFlagBits::eMultiInstanceKHR)
				);
			}
		}

		// Find highest score
		int max_val = std::numeric_limits<int>::min();
		size_t max_index = 0;
		for (size_t i = 0; i < physical_devices.size(); ++i)
		{
			if (device_scores[i] > max_val)
			{
				max_val = device_scores[i];
				max_index = i;
			}
		}

		m_physicalDevice = physical_devices[max_index];


		auto queue_family_properties = m_physicalDevice.getQueueFamilyProperties();

		// Find a graphics queue.
		size_t graphics_family_queue_index = 0ull;
		for (auto i = 0ull; i < queue_family_properties.size(); ++i)
		{
			if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				graphics_family_queue_index = i;
				break;
			}
		}

		size_t present_queue_family_index = 0ull;
		// Find a queue family that supports our surface
		for (auto i = 0ull; i < queue_family_properties.size(); ++i)
		{
			if (m_physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), m_surface.get()))
			{
				present_queue_family_index = i;
				break;
			}
		}

		// Our set of queue indicies we intend on using.
		m_uniqueQueueFamilyIndices =
		{
			static_cast<uint32_t>(graphics_family_queue_index),
			static_cast<uint32_t>(present_queue_family_index)
		};

		std::vector<uint32_t> family_indicies(m_uniqueQueueFamilyIndices.cbegin(), m_uniqueQueueFamilyIndices.cend());

		// A vector containing information required to create our queues
		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

		// Generate our create info
		float queue_priority = 0.0f;
		for (auto& queue_family_index : m_uniqueQueueFamilyIndices)
		{
			queue_create_infos.emplace_back(
				vk::DeviceQueueCreateFlags(),				// Flags
				static_cast<uint32_t>(queue_family_index),	// Index
				1,											// Queue count
				&queue_priority								// Queue priority
			);
		}

		// Required device extentions
		const std::vector<const char*> device_extentions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// Device create info
		vk::DeviceCreateInfo device_create_info(
			vk::DeviceCreateFlags(),							// Flags
			static_cast<uint32_t>(queue_create_infos.size()),	// Queue create info count
			queue_create_infos.data(),							// Queue create info data
			0u,													// Enabled layer count
			nullptr,											// Enabled layer names
			static_cast<uint32_t>(device_extentions.size()),	// Enabled extensions count
			device_extentions.data()							// Enabled extensions names
		);

		// Create the device
		m_device = m_physicalDevice.createDeviceUnique(device_create_info);

		// Utility struct
		struct SharingMode
		{
			vk::SharingMode sharing_mode;
			uint32_t family_indices_count;
			uint32_t* family_indicies_data;
		};

		// Handle case that there is only 1 queue in use.
		SharingMode	sharing_mode_util
		{
			(graphics_family_queue_index != present_queue_family_index) ?
				SharingMode{ vk::SharingMode::eConcurrent, 2u, family_indicies.data() } :
				SharingMode{ vk::SharingMode::eExclusive, 0u, nullptr}
		};


	}

	void VulkanApp::createSwapchain()
	{
		// Prepare validation warnings
		auto capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface);
		auto formats = m_physicalDevice.getSurfaceFormatsKHR(*m_surface);

		auto [width, height] = m_window->getSize();

		// 8 bits for blue, green, red, alpha
		auto format = vk::Format::eB8G8R8A8Unorm;
		auto extent = vk::Extent2D
		{ 
			static_cast<uint32_t>(width), 
			static_cast<uint32_t>(height) 
		};
	}
}