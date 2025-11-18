#ifndef SHADERPROGRAM_HPP
#define SHADERPROGRAM_HPP

#include <GL/glew.h>
#include <string>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    bool CreateFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    
    void Use() const;
    GLuint GetID() const { return programID; }
    
    // Uniform setters
    void SetUniformMatrix4fv(const std::string& name, const float* value) const;
    void SetUniform3f(const std::string& name, float x, float y, float z) const;
    void SetUniform1i(const std::string& name, int value) const;
    void SetUniform1f(const std::string& name, float value) const;

private:
    GLuint programID;
    
    GLuint CompileShader(const std::string& source, GLenum type) const;
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);
    std::string LoadShaderFromFile(const std::string& filepath) const;
};

#endif // SHADERPROGRAM_HPP