#include "window.hpp"

namespace sys
{
	Window::Window(int width, int height, const std::string& title) :
		m_title(title)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// Do not create an OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		// No resizing (for now)

		m_windowHandle.reset(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr));

		glfwSetWindowUserPointer(m_windowHandle.get(), this);
	}

	Window::~Window(){}

	// Statics
	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	void Window::initializeSystem()
	{
		glfwInit();
	}

	void Window::terminateSystem()
	{
		glfwTerminate();
	}

	// Status
	bool Window::shouldClose()  const
	{
		return glfwWindowShouldClose(m_windowHandle.get());
	}


	// Setters



	std::pair<int, int> Window::getSize() const
	{
		int width, height;
		glfwGetWindowSize(m_windowHandle.get(), &width, &height);
		return std::make_pair(width, height);
	}

	std::tuple<int, int, int, int> Window::getFrameSize() const
	{
		int left, top, right, bottom;
		glfwGetWindowFrameSize(m_windowHandle.get(), &left, &top, &right, &bottom);
		return std::make_tuple(left, top, right, bottom);
	}

	std::pair<int, int> Window::getPosition() const
	{
		int x, y;
		glfwGetWindowPos(m_windowHandle.get(), &x, &y);
		return std::make_pair(x, y);
	}

	std::string Window::getTitle() const
	{
		return m_title;
	}

	// Other

	vk::UniqueSurfaceKHR Window::createSurface(vk::Instance instance) const
	{
		VkSurfaceKHR tmp_surface;
		glfwCreateWindowSurface(instance, m_windowHandle.get(), nullptr, &tmp_surface);
		return vk::UniqueSurfaceKHR(tmp_surface, instance);
	}
}