#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"

Shader::Shader(const std::string& filepath) : m_FilePath(filepath), m_RendererID(0) {
	ShaderProgramSource gfx_shader = ParseShader(m_FilePath);
	m_RendererID = CreateShader(gfx_shader.VertexSource, gfx_shader.FragmentSource);
}

Shader::~Shader() {
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const {
	GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const {
	GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

// Private Methods

unsigned int Shader::GetUniformLocation(const std::string& name) {
    // uniform not found in cache
    std::unordered_map<std::string, int>::const_iterator found = m_UniformLocationCache.find(name);
    if (found == m_UniformLocationCache.end())
    {
        GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
        // shader uniform location not found
        if (location == -1) {
            std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
        }
        m_UniformLocationCache.insert({ name, location });
        return location;
    }
    else {
        return found->second;
    }
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    ShaderType type = ShaderType::NONE;
    std::string line;
    std::stringstream ss[2];

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                // set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                // set mode to fragment
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[int(type)] << line << '\n';
        }
    }

    ShaderProgramSource source;
    source.VertexSource = ss[int(ShaderType::VERTEX)].str();
    source.FragmentSource = ss[int(ShaderType::FRAGMENT)].str();

    return source; // Could also do { ss[0].str(), ss[1].str() } but keeping the above since more readable over time imo
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str(); // OpenGL expects source string in this format
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE) {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        // char message[length]; doesn't work allocating on the stack because c++ expects constant value to initialize array like char message[10]
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile shader " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}