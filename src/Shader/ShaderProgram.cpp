#include "ShaderProgram.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

ShaderProgram::ShaderProgram() : programID(0) {
}

ShaderProgram::~ShaderProgram() {
    if (programID) {
        glDeleteProgram(programID);
    }
}

bool ShaderProgram::CreateFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = LoadShaderFromFile(vertexPath);
    std::string fragmentSource = LoadShaderFromFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cout << "Failed to load shader files" << std::endl;
        return false;
    }
    
    GLuint vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    if (!vertexShader) return false;
    
    GLuint fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    bool success = LinkProgram(vertexShader, fragmentShader);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return success;
}

void ShaderProgram::Use() const {
    glUseProgram(programID);
}

void ShaderProgram::SetUniformMatrix4fv(const std::string& name, const float* value) const {
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void ShaderProgram::SetUniform3f(const std::string& name, float x, float y, float z) const {
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniform3f(location, x, y, z);
}

void ShaderProgram::SetUniform1i(const std::string& name, int value) const {
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniform1i(location, value);
}

void ShaderProgram::SetUniform1f(const std::string& name, float value) const {
    GLint location = glGetUniformLocation(programID, name.c_str());
    glUniform1f(location, value);
}

GLuint ShaderProgram::CompileShader(const std::string& source, GLenum type) const {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "Shader compile error (" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "): " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool ShaderProgram::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);
    
    GLint success = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(programID, 1024, NULL, infoLog);
        std::cout << "Shader link error: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

std::string ShaderProgram::LoadShaderFromFile(const std::string& filepath) const {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}