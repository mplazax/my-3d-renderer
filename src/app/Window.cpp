#include "app/Window.h"
#include <stdexcept>
#include <iostream>

// Static initialization
static bool s_glfwInitialized = false;

// Helper functions
static void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

Window::Window(int width, int height, const std::string& title) 
    : m_width(width), m_height(height) {
    
    // Initialize GLFW if not already done
    if (!s_glfwInitialized) {
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        s_glfwInitialized = true;
    }
    
    // Set OpenGL version and profile - crucial for macOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Make OpenGL context current
    glfwMakeContextCurrent(m_window);
    
    // Enable VSync
    glfwSwapInterval(1);
    
    // Set user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);
    
    // Set callbacks
    glfwSetKeyCallback(m_window, keyCallbackWrapper);
    glfwSetCursorPosCallback(m_window, mouseMoveCallbackWrapper);
    
    // Update actual framebuffer size (important for Retina displays)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    m_width = fbWidth;
    m_height = fbHeight;
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setKeyCallback(std::function<void(int, int, int, int)> callback) {
    m_keyCallback = std::move(callback);
}

void Window::setMouseMoveCallback(std::function<void(double, double)> callback) {
    m_mouseMoveCallback = std::move(callback);
}

int Window::getWidth() const {
    return m_width;
}

int Window::getHeight() const {
    return m_height;
}

float Window::getContentScaleFactor() const {
    float xscale, yscale;
    glfwGetWindowContentScale(m_window, &xscale, &yscale);
    return xscale; // Usually xscale == yscale, but we could average them
}

// Static callback wrappers
void Window::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowObj && windowObj->m_keyCallback) {
        windowObj->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::mouseMoveCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
    Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowObj && windowObj->m_mouseMoveCallback) {
        windowObj->m_mouseMoveCallback(xpos, ypos);
    }
} 