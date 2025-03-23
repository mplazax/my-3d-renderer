#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();
    
    bool compile(const std::string& vertexSource, const std::string& fragmentSource);
    bool compileFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    
    void bind();
    void unbind();
    
    // Uniform setters
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, const glm::vec2& value);
    void setUniform(const std::string& name, const glm::vec3& value);
    void setUniform(const std::string& name, const glm::vec4& value);
    void setUniform(const std::string& name, const glm::mat4& value);
    
    GLuint getId() const { return m_id; }
    
private:
    GLuint m_id;
    std::unordered_map<std::string, GLint> m_uniformLocations;
    
    GLint getUniformLocation(const std::string& name);
    bool compileShader(GLuint& shader, GLenum type, const std::string& source);
    std::string loadShaderFile(const std::string& path);
}; 