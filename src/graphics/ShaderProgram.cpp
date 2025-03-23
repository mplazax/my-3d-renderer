#include "graphics/ShaderProgram.h"
#include <iostream>
#include <fstream>
#include <sstream>

ShaderProgram::ShaderProgram() : m_id(0) {}

ShaderProgram::~ShaderProgram() {
    if (m_id != 0) {
        glDeleteProgram(m_id);
    }
}

bool ShaderProgram::compile(const std::string& vertexSource, const std::string& fragmentSource) {
    // Create program
    GLuint program = glCreateProgram();
    
    // Compile shaders
    GLuint vertexShader = 0, fragmentShader = 0;
    bool success = compileShader(vertexShader, GL_VERTEX_SHADER, vertexSource);
    if (!success) {
        glDeleteProgram(program);
        return false;
    }
    
    success = compileShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentSource);
    if (!success) {
        glDeleteShader(vertexShader);
        glDeleteProgram(program);
        return false;
    }
    
    // Attach shaders to program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    // Link program
    glLinkProgram(program);
    
    // Check link status
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog(infoLogLength, '\0');
        glGetProgramInfoLog(program, infoLogLength, nullptr, &infoLog[0]);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return false;
    }
    
    // Clean up shaders (they're linked into the program now)
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Store program ID
    if (m_id != 0) {
        glDeleteProgram(m_id);
    }
    m_id = program;
    
    // Clear uniform cache
    m_uniformLocations.clear();
    
    return true;
}

bool ShaderProgram::compileFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    // Load shader sources
    std::string vertexSource = loadShaderFile(vertexPath);
    std::string fragmentSource = loadShaderFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }
    
    return compile(vertexSource, fragmentSource);
}

void ShaderProgram::bind() {
    glUseProgram(m_id);
}

void ShaderProgram::unbind() {
    glUseProgram(0);
}

GLint ShaderProgram::getUniformLocation(const std::string& name) {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }
    
    GLint location = glGetUniformLocation(m_id, name.c_str());
    m_uniformLocations[name] = location;
    return location;
}

void ShaderProgram::setUniform(const std::string& name, int value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void ShaderProgram::setUniform(const std::string& name, float value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec2& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform2fv(location, 1, glm::value_ptr(value));
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(value));
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, glm::value_ptr(value));
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4& value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
}

bool ShaderProgram::compileShader(GLuint& shader, GLenum type, const std::string& source) {
    // Create shader
    shader = glCreateShader(type);
    
    // Set source
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    
    // Compile
    glCompileShader(shader);
    
    // Check compilation status
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog(infoLogLength, '\0');
        glGetShaderInfoLog(shader, infoLogLength, nullptr, &infoLog[0]);
        
        const char* shaderTypeStr = (type == GL_VERTEX_SHADER) ? "vertex" : 
                                   ((type == GL_FRAGMENT_SHADER) ? "fragment" : "unknown");
        std::cerr << "Shader compilation failed (" << shaderTypeStr << "): " << infoLog << std::endl;
        
        glDeleteShader(shader);
        shader = 0;
        return false;
    }
    
    return true;
}

std::string ShaderProgram::loadShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
} 