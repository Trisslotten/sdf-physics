#include "PhysicsSystem.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>
#include "engine/SystemManager.hpp"
#include "engine/shape/ShapeManager.hpp"
#include "engine/shape/ShapeMetadata.hpp"
#include "ecs/components/Transform.hpp"
#include "graphics/DebugDrawing.hpp"
#include "engine/Broadphase.hpp"
#include "Input.hpp"

struct PhysicsComponent {
	glm::vec2 velocity{};
	float angular_velocity = 0.f;
	glm::vec2 center_of_mass{};
	float mass = 0.f;
};

glm::vec2 CalculateVelocityAt(const PhysicsComponent& physics, const TransformComponent& transform, glm::vec2 position) {
	auto pivot = position - transform.position;
	return physics.velocity + physics.angular_velocity * glm::vec2(-pivot.y, pivot.x);
}

void ApplyImpulseAt(PhysicsComponent& physics, const TransformComponent& transform, glm::vec2 impulse, glm::vec2 position) {
	auto pivot = position - transform.position;
	physics.velocity += impulse / physics.mass;
	physics.angular_velocity += pivot.x * impulse.y - pivot.y * impulse.x;
}

PhysicsSystem::PhysicsSystem(SystemManager& system_manager)
	: m_entity_manager{ system_manager.Get<EntityManager>() }
	, m_shape_manager{ system_manager.Get<ShapeManager>() }
	, m_debug_drawing{ system_manager.Get<DebugDrawing>() }
	, m_input{ system_manager.Get<Input>() }
	, m_broadphase{ std::make_unique<Broadphase>(system_manager) }
{
	system_manager.OnUpdate().connect<&PhysicsSystem::Update>(this);
	system_manager.OnInitialize().connect<&PhysicsSystem::Initialize>(this);

	m_dragging_entity = m_entity_manager.invalid_entity();
}

PhysicsSystem::~PhysicsSystem() {}

void PhysicsSystem::Initialize() {
	srand(101);

	auto random = [](){ return rand() / float(RAND_MAX); };

	auto create_entity = [this](glm::vec2 pos, float rotation = 0.f){
		auto entity = m_entity_manager.create();

		auto size = glm::uvec2(64);
		auto& shape = m_shape_manager.CreateShape(Shape(size));
		m_entity_manager.emplace<ShapeId>(entity, shape.GetId());

		auto& transform = m_entity_manager.emplace<TransformComponent>(entity);
		transform.position = pos;
		transform.rotation = glm::two_pi<float>() * rotation;
		auto& physics = m_entity_manager.emplace<PhysicsComponent>(entity);
		physics.velocity = glm::vec2(0,0);
		physics.angular_velocity = 0.0f;

		auto& mass_values = GetMassValues(shape);
		shape.SetCenterOffset(mass_values.center_of_mass);

		physics.mass = mass_values.mass;
		physics.center_of_mass = mass_values.center_of_mass;
	};

	create_entity({2, 0.5}, random());
	create_entity({2.1, 1.2}, random());

	for (int i = 0; i < 50; ++i) {
		create_entity(glm::vec2(5, 15) * glm::vec2(random(), random()));
	}
}

void PhysicsSystem::Update(float deltatime) {
	//m_update_timer += deltatime;
	//constexpr float update_time = 1.f / 100.f;
	//constexpr float dt = 1.f / 1000.f;
	//if (m_update_timer > update_time) {
	//	m_update_timer -= update_time;
	//	m_debug_drawing.Clear();
	//} else {
	//	return;
	//}

	float dt = deltatime;
	//dt = glm::min(dt, 1.f/100.0f);
	m_debug_drawing.Clear();

	{
		auto view = m_entity_manager.view<TransformComponent, PhysicsComponent, ShapeId>().each();
		for (auto iter = view.begin(); iter != view.end(); ++iter) {
			auto [entity, transform, physics, shape_id] = *iter;
			auto& shape = *m_shape_manager.GetShape(shape_id);
			m_broadphase->AddDynamic(entity, transform, shape);
		}

		auto& intersections = m_broadphase->GetPotentiallyIntersections();
		for (auto& intersection : intersections) {
			auto entity_left = intersection.first;
			auto entity_right = intersection.second;

			auto [transform_left, physics_left, shape_id_left] =
				m_entity_manager.get<TransformComponent, PhysicsComponent, ShapeId>(entity_left);
			auto& shape_left = *m_shape_manager.GetShape(shape_id_left);
			auto& sdf_left = shape_left.GetSdf();


			auto [transform_right, physics_right, shape_id_right] =
				m_entity_manager.get<TransformComponent, PhysicsComponent, ShapeId>(entity_right);
			auto& shape_right = *m_shape_manager.GetShape(shape_id_right);
			auto& sdf_right = shape_right.GetSdf();

			// do box-box collision first

			auto rot_left = transform_left.CalculateRotationMatrix();
			auto rot_right = transform_right.CalculateRotationMatrix();
			auto inv_rot_left = glm::transpose(rot_left);
			auto inv_rot_right = glm::transpose(rot_right);

			glm::vec2 shape_corner_left = transform_left.position - rot_left * physics_left.center_of_mass;
			glm::vec2 shape_corner_right = transform_right.position - rot_right * physics_right.center_of_mass;

			glm::vec2 center = 0.5f * (transform_left.position + transform_right.position);
			glm::vec2 diff = transform_left.position - transform_right.position;
			glm::vec2 tangent = glm::normalize(glm::vec2(diff.y, -diff.x));

			auto size_left = shape_left.GetSizeInMeters();
			auto size_right = shape_right.GetSizeInMeters();
			auto start_step_scale = glm::min(glm::compMax(size_left), glm::compMax(size_right));

			std::array<glm::vec2, 2> march_positions{
				center + 0.2f * tangent,
				center - 0.2f * tangent
			};
			for (auto& march_pos : march_positions) {
				float step_size = 0.2f * start_step_scale;

				glm::vec2 prev_march_pos;
				float distance;
				glm::vec2 normal;
				glm::vec2 current_direction;
				glm::vec2 step_direction = glm::vec2(0);
				bool inside = false;
				constexpr int c_MaxSteps = 50;

				for (int i = 0; i < c_MaxSteps; ++i) {
					auto local_pos_left = (inv_rot_left * (march_pos - shape_corner_left));
					auto local_pos_right = (inv_rot_right * (march_pos - shape_corner_right));

					auto [distance_left, local_gradient_left] = sdf_left.GetDistanceAndGradient(local_pos_left);
					auto [distance_right, local_gradient_right] = sdf_right.GetDistanceAndGradient(local_pos_right);

					if (distance_left > distance_right) {
						distance = distance_left;
						current_direction = rot_left * local_gradient_left;
						normal = current_direction;
					} else {
						distance = distance_right;
						current_direction = rot_right * local_gradient_right;
						normal = -current_direction;
					}
					inside = distance <= 0.0f;

					step_direction = glm::mix(current_direction, step_direction, 0.5f);

					prev_march_pos = march_pos;
					march_pos += step_size * step_direction;

					//m_debug_drawing.AddLine(prev_march_pos, march_pos);

					if (inside && step_size < 0.5f * c_PixelSizeMeters) {
						break;
					}
					step_size *= 0.9f;
				}

				if (inside) {
					auto local_pos_left = (inv_rot_left * (march_pos - shape_corner_left));
					constexpr float c_eps = c_PixelSizeMeters;
					auto [_0, local_gradient_left] = sdf_left.GetDistanceAndGradient(local_pos_left + glm::vec2(-c_eps, 0));
					auto [_1, local_gradient_right] = sdf_left.GetDistanceAndGradient(local_pos_left + glm::vec2(c_eps, 0));
					auto [_2, local_gradient_up] = sdf_left.GetDistanceAndGradient(local_pos_left + glm::vec2(0, c_eps));
					auto [_3, local_gradient_down] = sdf_left.GetDistanceAndGradient(local_pos_left + glm::vec2(0, -c_eps));

					normal = inv_rot_left * (-normal) + local_gradient_left + local_gradient_right + local_gradient_up + local_gradient_down;
					normal *= 1.0f / 5.0f;
					normal = rot_left * normal;

					m_contacts.push_back(Contact{ entity_left, entity_right, march_pos, normal, 2.0f * glm::abs(distance) });

					m_debug_drawing.AddLine(march_pos, march_pos + 0.25f * normal);
					m_debug_drawing.AddCross(march_pos, 0.025f);
				}
			}
		}
	}

	auto view = m_entity_manager.view<TransformComponent, PhysicsComponent, ShapeId>().each();
	for (auto iter = view.begin(); iter != view.end(); ++iter) {
		auto [entity, transform, physics, shape_id] = *iter;
		auto& shape = *m_shape_manager.GetShape(shape_id);

		const auto size = shape.GetSizeInMeters();
		const auto radius = size.x;

		auto rotation = transform.CalculateRotationMatrix();
		glm::vec2 shape_corner = transform.position - rotation * physics.center_of_mass;

		constexpr float sqrt2 = 1.41421356237f;
		constexpr float c_PixelRadius = 1.41421356237f * 0.5f * c_PixelSizeMeters;

		auto plane = [&](glm::vec2 pos, glm::vec2 normal) {
			float origin = glm::dot(pos, normal);
			float projected_center = glm::dot(transform.position, normal);
			float point = projected_center - radius;
			if (point <= origin) {
				auto& usize = shape.GetSize();
				glm::uvec2 i;
				for (i.y = 0; i.y < usize.y; ++i.y) {
					for (i.x = 0; i.x < usize.x; ++i.x) {
						auto pixel = shape.GetPixelAt(i);
						if (pixel != c_MaterialEmptySpace) {
							const glm::vec2 local_pos = c_PixelSizeMeters * (glm::vec2(i) + 0.5f);
							auto collision_pos = shape_corner + rotation * local_pos;

							float projected = glm::dot(collision_pos, normal);
							if (projected - c_PixelRadius <= origin) {
								float overlap = origin - (projected - c_PixelRadius);
								collision_pos -= normal * c_PixelRadius;

								m_contacts.push_back(Contact{ entity, m_entity_manager.invalid_entity(), collision_pos, normal, overlap });
								//m_debug_drawing.AddLine(collision_pos, collision_pos + normal);
							}

							//constexpr float bog = 0.25f * c_PixelSizeMeters;
							//m_debug_drawing.AddLine(collision_pos + glm::vec2(bog, 0), collision_pos - glm::vec2(bog, 0));
							//m_debug_drawing.AddLine(collision_pos + glm::vec2(0, bog), collision_pos - glm::vec2(0, bog));
						}
					}
				}
			}
		};

		plane(glm::vec2(0,0), glm::vec2(0,1));
		plane(glm::vec2(0,0), glm::vec2(1,0));
		plane(glm::vec2(6,0), glm::vec2(-1,0));
	}

	constexpr int c_MaxSolverIterations = 5;
	for (int i = 0; i < c_MaxSolverIterations; ++i) {
		for (auto& contact : m_contacts) {
			auto* physics_left = m_entity_manager.try_get<PhysicsComponent>(contact.entity_left);
			auto* transform_left = m_entity_manager.try_get<TransformComponent>(contact.entity_left);

			auto* physics_right = m_entity_manager.try_get<PhysicsComponent>(contact.entity_right);
			auto* transform_right = m_entity_manager.try_get<TransformComponent>(contact.entity_right);

			float normal_mass = 0.f;
			glm::vec2 relative_vel(0.f);
			if (physics_left) {
				normal_mass += 1.0f / physics_left->mass;
				relative_vel += CalculateVelocityAt(*physics_left, *transform_left, contact.position);
			}
			if (physics_right) {
				normal_mass += 1.0f / physics_right->mass;
				relative_vel -= CalculateVelocityAt(*physics_right, *transform_right, contact.position);
			}

			const float normal_vel = glm::dot(contact.normal, relative_vel);
			//float restitution = 0.3f;
			auto impulse = normal_vel;
			impulse -= 0.01f * glm::max(contact.intersection_depth - 0.5f * c_PixelSizeMeters, 0.f) / dt;
			impulse /= normal_mass;
			float old_impulse = contact.previous_impulse;
			contact.previous_impulse = glm::min(impulse + old_impulse, 0.f);
			impulse = contact.previous_impulse - old_impulse;

			const auto tangent = glm::vec2(contact.normal.y, -contact.normal.x);
			const float tangent_vel = glm::dot(tangent, relative_vel);
			auto tangent_impulse = tangent_vel / normal_mass;

			constexpr float c_Friction = 0.2f;
			float old_tangent_impulse = contact.previous_tangent_impulse;
			float clamp_value = glm::abs(c_Friction * contact.previous_impulse);
			contact.previous_tangent_impulse = glm::clamp(tangent_impulse + old_tangent_impulse, -clamp_value, clamp_value);
			tangent_impulse = contact.previous_tangent_impulse - old_tangent_impulse;

			const glm::vec2 impulse_vec = impulse * contact.normal + tangent_impulse * tangent;
			if (transform_left && physics_left) {
				ApplyImpulseAt(*physics_left, *transform_left, -impulse_vec, contact.position);
			}
			if (transform_right && physics_right) {
				ApplyImpulseAt(*physics_right, *transform_right, impulse_vec, contact.position);
			}
		}
	}

	m_contacts.clear();

	for (auto &&[entity, transform, physics] : m_entity_manager.view<TransformComponent, PhysicsComponent>().each()) {
		//m_debug_drawing.AddCross(transform.position, 0.1f);

		transform.position += physics.velocity * dt;
		transform.rotation += physics.angular_velocity * dt;

		physics.velocity /= 1.0f + 0.1f * dt;
		physics.angular_velocity /= 1.0f + 0.1f * dt;

		physics.velocity.y -= 9.82 * dt;
	}
}

MassValues& PhysicsSystem::GetMassValues(const Shape& shape) {
	if (auto iter = m_mass_values.find(shape.GetId()); iter != m_mass_values.end()) {
		return iter->second;
	} else {
		auto& size = shape.GetSize();

		MassValues result;

		glm::uvec2 i;
		for (i.y = 0; i.y < size.y; ++i.y) {
			for (i.x = 0; i.x < size.x; ++i.x) {
				auto pixel = shape.GetPixelAt(i);
				if (pixel != c_MaterialEmptySpace) {
					constexpr float density = 100.0f;
					float mass = c_PixelAreaMeters * density;
					glm::vec2 current_pos = c_PixelSizeMeters * (glm::vec2(i) + 0.5f);
					result.center_of_mass += mass * current_pos;
					result.mass += mass;
				}
			}
		}
		result.center_of_mass /= result.mass;

		{
			auto iter = m_mass_values.emplace(shape.GetId(), std::move(result));
			return iter.first->second;
		}
	}
}
