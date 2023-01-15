#pragma once

#include <memory>
#include "engine/System.hpp"
#include "shader.hpp"
#include "Camera.hpp"
#include "util/IntTypes.hpp"
#include "ecs/EntityManager.hpp"

class Window;
class SystemManager;
class ShapeManager;
class ShapeTextureManager;
class DebugDrawing;

namespace Graphics {

class SquareMesh {
public:
	void Initialize();
	void Draw();
private:
	u32 m_vao = 0;
	u32 m_vbo = 0;
};

class LineMesh {
public:
	void Initialize();
	void Draw();
private:
	u32 m_vao = 0;
	u32 m_vbo = 0;
};

class Renderer final : public System {
public:
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	Renderer(SystemManager& system_manager);
	~Renderer();

	void SetCamera(const Camera& camera);
	void ReloadShaders();

	void Render(float);
private:
	void Initialize();
	void OnShapeCreated(entt::registry&, entt::entity);

	void SetCameraUniforms(ShaderProgram& shader);
	float GetAspectRatio() const;

	EntityManager& m_entity_manager;
	ShapeManager& m_shape_manager;
	DebugDrawing& m_debug_drawing;
	std::unique_ptr<ShapeTextureManager> m_shape_texture_manager;

	i32 m_window_width = 1280;
	i32 m_window_height = 720;

	ShaderProgram m_background_shader;
	ShaderProgram m_object_shader;
	ShaderProgram m_debug_line_shader;

	SquareMesh m_square_mesh;
	LineMesh m_line_mesh;

	Camera m_camera;
};

}
