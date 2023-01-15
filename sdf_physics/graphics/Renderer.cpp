#include "Renderer.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/color_space.hpp>

#include "window.hpp"
#include "engine/SystemManager.hpp"
#include "ecs/components/Transform.hpp"
#include "engine/shape/ShapeId.hpp"
#include "engine/shape/ShapeManager.hpp"
#include "ShapeTextureManager.hpp"
#include "engine/shape/ShapeMetadata.hpp"
#include "DebugDrawing.hpp"

using namespace Graphics;

void SquareMesh::Initialize() {
	float vertices[] = {
		0.f, 0.f,
		0.f, 1.f,
		1.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	glBindVertexArray(0);
}

void SquareMesh::Draw() {
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void LineMesh::Initialize() {
	float vertices[] = { 0.f, 1.f };

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	glBindVertexArray(0);
}

void LineMesh::Draw() {
	glBindVertexArray(m_vao);
	glDrawArrays(GL_LINES, 0, 2);
}

Renderer::Renderer(SystemManager& system_manager)
	: m_entity_manager{ system_manager.Get<EntityManager>() }
	, m_shape_manager{ system_manager.Get<ShapeManager>() }
	, m_shape_texture_manager{ std::make_unique<ShapeTextureManager>() }
	, m_debug_drawing{ system_manager.Get<DebugDrawing>() }
{
	auto& window = system_manager.Get<Window>();

	window.RegisterResizedCallback([this](i32 w, i32 h) {
		m_window_width = w;
		m_window_height = h;
		glViewport(0, 0, w, h);
	});

	system_manager.OnInitialize().connect<&Renderer::Initialize>(this);
	system_manager.OnUpdate().connect<&Renderer::Render>(this);

	m_entity_manager.on_construct<ShapeId>().connect<&Renderer::OnShapeCreated>(this);
}

Renderer::~Renderer() = default;

void Renderer::SetCamera(const Camera& camera) {
	m_camera = camera;
}

void Renderer::ReloadShaders() {
	m_background_shader.Reload();
}

void Renderer::Initialize() {
	glClearColor(216.f/255.f, 242.f / 255.f, 255.f / 255.f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	m_square_mesh.Initialize();
	m_line_mesh.Initialize();

	{
		m_background_shader.Add("background.vert");
		m_background_shader.Add("background.frag");
		m_background_shader.Compile();

		m_object_shader.Add("shader.vert");
		m_object_shader.Add("shader.frag");
		m_object_shader.Compile();

		m_debug_line_shader.Add("line.vert");
		m_debug_line_shader.Add("line.frag");
		m_debug_line_shader.Compile();
	}
}

void Renderer::SetCameraUniforms(ShaderProgram& shader) {
	shader.Uniform("view_position", m_camera.position);

	glm::vec2 scale = glm::vec2(1.f / m_camera.zoom);
	scale.y *= GetAspectRatio();

	shader.Uniform("view_scale", scale);
	//shader.Uniform("view_rotation", m_camera.rotation);
}

float Renderer::GetAspectRatio() const {
	return static_cast<float>(m_window_width) / m_window_height;
}

void Renderer::OnShapeCreated(entt::registry& registry, entt::entity entity) {
	ShapeId shape_id = registry.get<ShapeId>(entity);
	if (auto* shape = m_shape_manager.GetShape(shape_id)) {
		m_shape_texture_manager->CreateTexture(shape_id, *shape);
	}
}

void Renderer::Render(float dt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		m_debug_line_shader.Use();

		SetCameraUniforms(m_debug_line_shader);

		for (auto& line : m_debug_drawing.GetLines()) {
			m_debug_line_shader.Uniform("line_start", line.start);
			m_debug_line_shader.Uniform("line_end", line.end);

			m_line_mesh.Draw();
		}

		//m_debug_drawing.Clear();
	}

	{
		m_object_shader.Use();
		SetCameraUniforms(m_object_shader);

		for (auto &&[entity, transform, shape_id] : m_entity_manager.view<TransformComponent, ShapeId>().each()) {
			auto* shape = m_shape_manager.GetShape(shape_id);
			auto* texture = m_shape_texture_manager->GetTexture(shape_id);

			if (shape && texture) {
				glm::vec2 size = c_PixelSizeMeters * glm::vec2(texture->size);
				texture->Bind(0);

				auto rotation = transform.CalculateRotationMatrix();

				m_object_shader.Uniform("u_mat_tex", 0);
				m_object_shader.Uniform("u_size", size);
				m_object_shader.Uniform("u_corner", transform.position - rotation * shape->GetCenterOffset());
				m_object_shader.Uniform("u_rotation", transform.rotation);

				float idf = glm::mod(2000.f * static_cast<float>(shape_id.Value()), 360.f);
				glm::vec3 color = glm::rgbColor(glm::vec3(idf, 1.0f, 1.0f));
				m_object_shader.Uniform("u_color", color);
				m_square_mesh.Draw();
			}
		}
	}

	{
		m_background_shader.Use();
		SetCameraUniforms(m_background_shader);

		m_square_mesh.Draw();
	}
}
