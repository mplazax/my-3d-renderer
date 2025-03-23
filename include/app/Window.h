#pragma once
#include <string>
#include <functional>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    void setMouseMoveCallback(std::function<void(double, double)> callback);
    
    int getWidth() const;
    int getHeight() const;
    float getContentScaleFactor() const; // For Retina displays
    
    GLFWwindow* getGLFWWindow() { return m_window; }
    
private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    
    // Callbacks
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(double, double)> m_mouseMoveCallback;
    
    // GLFW callback wrappers
    static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseMoveCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
}; 