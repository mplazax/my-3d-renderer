#include <iostream>
#include <memory>
#include <string>
#include <glad/glad.h>

#include "app/Window.h"
#include "graphics/ShaderProgram.h"

// Placeholder for future components
// #include "core/MemoryPool.h"
// #include "scene/Camera.h"
// #include "scene/Mesh.h"

void initializeOpenGL() {
    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    
    // Print OpenGL info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Check that we have OpenGL 4.1
    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    if (major < 4 || (major == 4 && minor < 1)) {
        throw std::runtime_error("OpenGL 4.1 is required but not available");
    }
    
    // Set common OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

// Simple shader for testing
const char* basicVertexShader = R"(
#version 410 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

out vec3 vColor;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

const char* basicFragmentShader = R"(
#version 410 core
in vec3 vColor;
out vec4 fragColor;

void main() {
    fragColor = vec4(vColor, 1.0);
}
)";

int main() {
    try {
        // Create window
        Window window(1280, 720, "LowLevelRenderer");
        
        // Initialize OpenGL
        initializeOpenGL();
        
        // Create shader program
        ShaderProgram shader;
        if (!shader.compile(basicVertexShader, basicFragmentShader)) {
            throw std::runtime_error("Failed to compile shaders");
        }
        
        // Simple triangle for testing
        struct Vertex {
            float position[3];
            float color[3];
        };
        
        Vertex vertices[] = {
            {{ -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }},
            {{ 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
            {{ 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }}
        };
        
        // Create vertex buffer
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        // Color attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
        
        // Main loop
        while (!window.shouldClose()) {
            // Clear screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Set up identity matrices for demo
            glm::mat4 model(1.0f);
            glm::mat4 view(1.0f);
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                                    (float)window.getWidth() / window.getHeight(), 
                                                    0.1f, 100.0f);
            
            // Bind shader and set uniforms
            shader.bind();
            shader.setUniform("uModel", model);
            shader.setUniform("uView", view);
            shader.setUniform("uProjection", projection);
            
            // Draw triangle
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
            
            // Swap buffers and poll events
            window.swapBuffers();
            window.pollEvents();
        }
        
        // Clean up
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 