#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "input.hpp"
#include "engine/SystemManager.hpp"

class WindowCallbacks {
public:
   static void ErrorCallback(i32 error, const char* description) {
      std::cerr << "GLFW Error: " << description << std::endl;
   }
   static void FrameBufferSizeCallback(GLFWwindow* glfw_window, i32 width, i32 height) {
      Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

      for (const auto& callback : window->m_resized_callbacks) {
         callback(width, height);
      }
   }
   static void WindowFocusCallback(GLFWwindow* glfw_window, int focused) {
      Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

      //if (focused) {
      //   Window::SetMouseLocked(mouse_was_locked);
      //} else {
      //   mouse_was_locked = mouse_locked;
      //   Window::SetMouseLocked(false);
      //}
   }
private:
   WindowCallbacks() = default;
};


Window::Window(SystemManager& system_manager, i32 width, i32 height) {
	system_manager.OnUpdate().connect<&Window::Update>(this);

   glfwSetErrorCallback(&WindowCallbacks::ErrorCallback);

   if (!glfwInit()) {
      // error
      return;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 8);

   m_glfw_window = glfwCreateWindow(width, height, "sdf physics", NULL, NULL);

   if (!m_glfw_window) {
      glfwTerminate();
      // error
      return;
   }
   glfwSetWindowUserPointer(m_glfw_window, this);
   glfwMakeContextCurrent(m_glfw_window);

	m_input = &system_manager.Add<Input>(m_glfw_window);

   if (!gladLoadGL()) {
      glfwTerminate();
      // error
      return;
   }

   std::cout << "Using OpenGL " << glGetString(GL_VERSION) << "\n";

   //glfwSetInputMode(m_glfw_window, GLFW_STICKY_KEYS, GLFW_TRUE);
   glfwSetFramebufferSizeCallback(m_glfw_window, &WindowCallbacks::FrameBufferSizeCallback);
   glfwSetWindowFocusCallback(m_glfw_window, &WindowCallbacks::WindowFocusCallback);
   glfwSwapInterval(1);
}

Window::~Window() {
   glfwTerminate();
   m_glfw_window = nullptr;
};

void Window::RegisterResizedCallback(ResizeCallback callback) {
   i32 width;
   i32 height;
   glfwGetWindowSize(m_glfw_window, &width, &height);
   callback(width, height);
   m_resized_callbacks.push_back(callback);
}

glm::ivec2 Window::GetWindowSize() const {
	i32 width;
	i32 height;
	glfwGetWindowSize(m_glfw_window, &width, &height);
	return glm::ivec2(width, height);
}

bool Window::ShouldClose() const {
   if (m_glfw_window) {
      return glfwWindowShouldClose(m_glfw_window);
   }
   return true;
}

void Window::Update(float dt) {
   if (m_glfw_window) {
      m_input->Update();
      glfwSwapBuffers(m_glfw_window);
      glfwPollEvents();
   }
}
