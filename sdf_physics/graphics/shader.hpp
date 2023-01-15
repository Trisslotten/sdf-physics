#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <robin_hood/robin_hood.h>

#define SHADER_UNIFORM_DECLERATION(glmtype) \
  void Uniformv(const std::string& name, GLuint count, const glmtype* values); \
  void Uniform(const std::string& name, const glmtype& value);

namespace Graphics {

class ShaderProgram {
public:
	ShaderProgram();
	~ShaderProgram();

	void Add(GLenum type, const std::string& path);
	void Add(const std::string& path);
	void Compile();
	void Reload();
	void Use();
	bool IsCompiled() { return m_is_compiled; }

	SHADER_UNIFORM_DECLERATION(GLfloat)
	SHADER_UNIFORM_DECLERATION(glm::vec2)
	SHADER_UNIFORM_DECLERATION(glm::vec3)
	SHADER_UNIFORM_DECLERATION(glm::vec4)

	SHADER_UNIFORM_DECLERATION(GLint)
	SHADER_UNIFORM_DECLERATION(glm::ivec2)
	SHADER_UNIFORM_DECLERATION(glm::ivec3)
	SHADER_UNIFORM_DECLERATION(glm::ivec4)

	SHADER_UNIFORM_DECLERATION(GLuint)
	SHADER_UNIFORM_DECLERATION(glm::uvec2)
	SHADER_UNIFORM_DECLERATION(glm::uvec3)
	SHADER_UNIFORM_DECLERATION(glm::uvec4)

	SHADER_UNIFORM_DECLERATION(glm::mat2)
	SHADER_UNIFORM_DECLERATION(glm::mat3)
	SHADER_UNIFORM_DECLERATION(glm::mat4)

	SHADER_UNIFORM_DECLERATION(glm::mat2x3)
	SHADER_UNIFORM_DECLERATION(glm::mat3x2)

	SHADER_UNIFORM_DECLERATION(glm::mat2x4)
	SHADER_UNIFORM_DECLERATION(glm::mat4x2)

	SHADER_UNIFORM_DECLERATION(glm::mat3x4)
	SHADER_UNIFORM_DECLERATION(glm::mat4x3)

private:
	std::unordered_multimap<GLenum, std::string> m_paths;

	std::unordered_multimap<GLenum, GLuint> m_shader_ids;
	GLuint m_program_id;

	GLuint FindUniformLocation(const std::string& name);

	robin_hood::unordered_flat_map<std::string, GLuint> m_uniform_locations;

	bool m_is_compiled = false;
};

}

#undef SHADER_UNIFORM_DECLERATION