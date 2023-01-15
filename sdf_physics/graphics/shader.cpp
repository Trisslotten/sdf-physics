#include "shader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

using namespace Graphics;

const std::string SHADERS_PATH = "graphics/shaders/";

#define SHADER_UNIFORM_DEFINITION(glFunc, glmtype, basetype)                   \
  void ShaderProgram::Uniformv(const std::string& name, GLuint count,          \
                               const glmtype* values) {                        \
    glFunc(FindUniformLocation(name), count, (basetype*)values);               \
  }                                                                            \
  void ShaderProgram::Uniform(const std::string& name, const glmtype& value) { \
    Uniformv(name, 1, &value);                                                 \
  }

#define SHADER_MATRIX_UNIFORM_DEFINITION(glFunc, glmtype, basetype)            \
  void ShaderProgram::Uniformv(const std::string& name, GLuint count,          \
                               const glmtype* values) {                        \
    glFunc(FindUniformLocation(name), count, GL_FALSE, (basetype*)values);     \
  }                                                                            \
  void ShaderProgram::Uniform(const std::string& name, const glmtype& value) { \
    Uniformv(name, 1, &value);                                                 \
  }

bool CheckLinkError(GLuint id, const std::string& paths) {
	GLint success = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		GLint log_size = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_size);
		if (log_size > 0) {
			std::vector<GLchar> error(log_size);
			glGetProgramInfoLog(id, log_size, &log_size, &error[0]);
			std::string errorstr{ &error[0] };

			std::cout << "Error in:\n" << paths << "\n" << errorstr << "\n";

			glDeleteProgram(id);
		}
		return false;
	} else {
		return true;
	}
}

GLuint CompileShader(GLenum type, const std::string& name) {
	GLuint shader = glCreateShader(type);

	std::string path = name;
	std::ifstream shader_file(path);
	std::string shader_text((std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>());
	if (!shader_file.is_open()) {
		std::cout << "ERROR Could not open file: " << path << "\n";
	}

	shader_file.close();
	const char* shaderTextPtr = shader_text.c_str();

	glShaderSource(shader, 1, &shaderTextPtr, nullptr);
	glCompileShader(shader);

	// Check for compile error
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		GLint log_size = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		std::vector<GLchar> error(log_size);
		glGetShaderInfoLog(shader, log_size, &log_size, &error[0]);
		std::string errorstr{ &error[0] };

		std::cout << "ERROR compiling shader '" << name << "':\n"
			<< errorstr << "\n";

		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

ShaderProgram::ShaderProgram() { m_program_id = 0; }

ShaderProgram::~ShaderProgram() {
	for (auto& i : m_shader_ids) {
		glDeleteShader(i.second);
	}
	glDeleteProgram(m_program_id);
}

void ShaderProgram::Add(GLenum type, const std::string& path) {
	// glDeleteShader(ids[type]);
	m_paths.insert(std::make_pair(type, SHADERS_PATH + path));
}

void ShaderProgram::Add(const std::string& path) {
	std::string extension = path.substr(path.size() - 4, 4);

	GLenum shader_type = GL_FRAGMENT_SHADER;

	if (extension == "vert") shader_type = GL_VERTEX_SHADER;
	if (extension == "tesc") shader_type = GL_TESS_CONTROL_SHADER;
	if (extension == "tese") shader_type = GL_TESS_EVALUATION_SHADER;
	if (extension == "geom") shader_type = GL_GEOMETRY_SHADER;
	if (extension == "frag") shader_type = GL_FRAGMENT_SHADER;
	if (extension == "comp") shader_type = GL_COMPUTE_SHADER;

	Add(shader_type, path);
}

void ShaderProgram::Compile() {
	std::string paths_str;

	std::unordered_multimap<GLenum, GLuint> temp_ids;
	GLuint temp_id;

	bool success = true;

	for (auto& path : m_paths) {
		GLuint shader_id = CompileShader(path.first, path.second);
		if (shader_id != 0) {
			temp_ids.insert(std::make_pair(path.first, shader_id));
		} else {
			success = false;
		}
		paths_str += path.second + "\n";
	}

	if (success) {
		temp_id = glCreateProgram();
		for (auto& i : temp_ids) {
			glAttachShader(temp_id, i.second);
		}
		glLinkProgram(temp_id);

		if (CheckLinkError(temp_id, paths_str)) {
			m_program_id = temp_id;
			m_shader_ids = temp_ids;
			m_is_compiled = true;
		}
	}
}

void ShaderProgram::Reload() {
	/*
	  for (auto elem : ids) glDeleteShader(elem.second);
	  glDeleteProgram(id);
	*/
	std::cout << "DEBUG shader.cpp: Reloading Shaders:\n";
	for (const auto& elem : m_paths) {
		std::cout << "  " << elem.second << "\n";
	}
	m_uniform_locations.clear();

	Compile();
}

void ShaderProgram::Use() { glUseProgram(m_program_id); }

GLuint ShaderProgram::FindUniformLocation(const std::string& name) {
	auto it = m_uniform_locations.find(name);
	GLuint uniform_location;
	if (it == m_uniform_locations.end()) {
		uniform_location = glGetUniformLocation(m_program_id, name.c_str());
		if (uniform_location == -1) {
			std::cout << "WARNING shader.cpp: could not find uniform '" << name
				<< "' in shaders: \n";
			for (auto item : m_paths) {
				std::cout << "\t" << item.second << "\n";
			}
		}
		m_uniform_locations[name] = uniform_location;
	} else
		uniform_location = it->second;
	return uniform_location;
}

SHADER_UNIFORM_DEFINITION(glUniform1fv, GLfloat, GLfloat)
SHADER_UNIFORM_DEFINITION(glUniform2fv, glm::vec2, GLfloat)
SHADER_UNIFORM_DEFINITION(glUniform3fv, glm::vec3, GLfloat)
SHADER_UNIFORM_DEFINITION(glUniform4fv, glm::vec4, GLfloat)

SHADER_UNIFORM_DEFINITION(glUniform1iv, GLint, GLint)
SHADER_UNIFORM_DEFINITION(glUniform2iv, glm::ivec2, GLint)
SHADER_UNIFORM_DEFINITION(glUniform3iv, glm::ivec3, GLint)
SHADER_UNIFORM_DEFINITION(glUniform4iv, glm::ivec4, GLint)

SHADER_UNIFORM_DEFINITION(glUniform1uiv, GLuint, GLuint)
SHADER_UNIFORM_DEFINITION(glUniform2uiv, glm::uvec2, GLuint)
SHADER_UNIFORM_DEFINITION(glUniform3uiv, glm::uvec3, GLuint)
SHADER_UNIFORM_DEFINITION(glUniform4uiv, glm::uvec4, GLuint)

SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix2fv, glm::mat2, GLfloat)
SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix3fv, glm::mat3, GLfloat)
SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix4fv, glm::mat4, GLfloat)

SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix2x3fv, glm::mat2x3, GLfloat)
SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix3x2fv, glm::mat3x2, GLfloat)

SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix2x4fv, glm::mat2x4, GLfloat)
SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix4x2fv, glm::mat4x2, GLfloat)

SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix3x4fv, glm::mat3x4, GLfloat)
SHADER_MATRIX_UNIFORM_DEFINITION(glUniformMatrix4x3fv, glm::mat4x3, GLfloat)