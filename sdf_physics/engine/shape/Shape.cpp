#include "Shape.hpp"

#include <glm/gtc/noise.hpp>
#include <glm/gtx/component_wise.hpp>
#include <time.h>
#include "ShapeMetadata.hpp"
#include <glm/gtx/norm.hpp>

Shape::Shape() {
	m_size = glm::uvec2(128);

	GenerateRandomShape();
}

Shape::Shape(glm::uvec2 size) {
	m_size = size;

	GenerateRandomShape();
}

const glm::uvec2& Shape::GetSize() const {
	return m_size;
}

const std::vector<u8>& Shape::GetImage() const {
	return m_image;
}

u8 Shape::GetPixelAt(glm::uvec2 pixel) const {
	return m_image[pixel.x + pixel.y * m_size.x];
}

glm::vec2 Shape::GetSizeInMeters() const {
	return c_PixelSizeMeters * glm::vec2(m_size);
}

void Shape::SetCenterOffset(glm::vec2 offset) {
	m_center_offset = offset;
}

const glm::vec2& Shape::GetCenterOffset() const {
	return m_center_offset;
}

const ShapeSdf& Shape::GetSdf() const {
	return m_sdf;
}

ShapeId Shape::GetId() const {
	return m_id;
}

void Shape::SetId(ShapeId id) {
	m_id = id;
}

void Shape::GenerateRandomShape() {
	m_image.resize(glm::compMul(m_size));

	auto rand_x = 0.03f * (rand() % 2000);
	auto rand_y = 0.03f * (rand() % 2000);

	glm::vec2 center = glm::vec2(m_size) * 0.5f;
	float radius = center.x;
	for (u32 y = 0; y < m_size.y; ++y) {
		for (u32 x = 0; x < m_size.x; ++x) {
			auto pos = glm::vec2(x, y);
			float noise = 0.5f + 0.5f * glm::simplex(0.02f * pos + glm::vec2(rand_x, rand_y));
			//noise = 1.0f;

			noise *= glm::smoothstep(radius, radius*0.5f, glm::length(radius - pos));

			auto& pixel = m_image[x + y * m_size.x];
			if (noise > 0.5f) {
				pixel = 1;
			} else {
				pixel = c_MaterialEmptySpace;
			}
		}
	}

	m_sdf.Create(m_image, m_size);
}

void ShapeSdf::Create(const std::vector<u8>& image, glm::uvec2 size) {
	m_size = size;

	// From http://www.codersnotes.com/notes/signed-distance-fields/

	struct Point {
		glm::ivec2 delta{ 9999, 9999 };
		int SquaredDistance() const {
			return delta.x * delta.x + delta.y * delta.y;
		}
	};
	class Grid {
	public:
		Grid(glm::uvec2 size) {
			this->size = size;
			points.resize(size.x * size.y);
		}
		Point Get(glm::ivec2 index) {
			if (index.x >= 0 && index.y >= 0 && index.x < size.x && index.y < size.y) {
				return points[index.x + index.y * size.x];
			} else {
				return {};
			}
		}
		void Set(glm::ivec2 index, Point point) {
			points[index.x + index.y * size.x] = point;
		}
		void GenerateSdf() {
			for (i32 y = 0; y < size.y; ++y) {
				for (i32 x = 0; x < size.x; ++x) {
					Point point = Get({x, y});
					Compare(point, {x,y}, {-1, 0});
					Compare(point, {x,y}, {0, -1});
					Compare(point, {x,y}, {-1, -1});
					Compare(point, {x,y}, {1, -1});
					Set({x, y}, point);
				}
				for (i32 x = size.x-1; x >= 0; --x) {
					Point point = Get({x, y});
					Compare(point, {x,y}, {1, 0});
					Set({x, y}, point);
				}
			}

			for (i32 y = size.y-1; y >= 0; --y) {
				for (i32 x = size.x-1; x >= 0; --x) {
					Point point = Get({x, y});
					Compare(point, {x,y}, {1, 0});
					Compare(point, {x,y}, {0, 1});
					Compare(point, {x,y}, {-1, 1});
					Compare(point, {x,y}, {1, 1});
					Set({x, y}, point);
				}
				for (i32 x = 0; x < size.x; ++x) {
					Point point = Get({x, y});
					Compare(point, {x,y}, {-1, 0});
					Set({x, y}, point);
				}
			}
		}
	private:
		void Compare(Point& point, glm::ivec2 index, glm::ivec2 offset) {
			Point other = Get(index + offset);
			other.delta += offset;
			if (other.SquaredDistance() < point.SquaredDistance()) {
				point = other;
			}
		}
		std::vector<Point> points;
		glm::uvec2 size;
	};

	Grid outside_grid{ size };
	Grid inside_grid{ size };

	for (u32 y = 0; y < size.y; ++y) {
		for (u32 x = 0; x < size.x; ++x) {
			auto material = image[x + y * size.x];

			if (material == c_MaterialEmptySpace) {
				inside_grid.Set({ x, y }, { {0, 0} });
			} else {
				outside_grid.Set({ x, y }, { {0, 0} });
			}
		}
	}

	outside_grid.GenerateSdf();
	inside_grid.GenerateSdf();

	//std::vector<float> distances;
	//distances.resize(size.x * size.y);
	m_distances.resize(size.x * size.y);


	for (u32 y = 0; y < size.y; ++y) {
		for (u32 x = 0; x < size.x; ++x) {
			float outside_distance = glm::sqrt(outside_grid.Get({x, y}).SquaredDistance());
			float inside_distance = glm::sqrt(inside_grid.Get({x, y}).SquaredDistance());

			if (outside_distance > 0.f) {
				outside_distance -= 0.5f;
			}
			if (inside_distance > 0.f) {
				inside_distance -= 0.5f;
			}
			float signed_distance = outside_distance - inside_distance;

			m_distances[x + y * size.x] = signed_distance;
		}
	}

	auto copy = m_distances;

	m_min_distance = std::numeric_limits<float>::max();
	m_max_distance = std::numeric_limits<float>::lowest();

	glm::mat3 kernel = {
		1,2,1,
		2,4,2,
		1,2,1,
		//1,1,1,
		//1,1,1,
		//1,1,1,
	};
	kernel *= 1.f / 16.f;
	//kernel *= 1.f / 9.f;
	for (i32 y = 0; y < size.y; ++y) {
		for (i32 x = 0; x < size.x; ++x) {
			float sum = 0.f;
			for (i32 yo = -1; yo <= 1; ++yo) {
				for (i32 xo = -1; xo <= 1; ++xo) {
					auto index = glm::clamp(glm::ivec2(x+xo, y+yo), glm::ivec2(0), glm::ivec2(m_size)-1);
					sum += kernel[yo+1][xo+1] * copy[index.x + index.y * m_size.x];
				}
			}
			sum *= 1.f / 9.f;

			m_distances[x + y * m_size.x] = sum;

			m_max_distance = glm::max(sum, m_max_distance);
			m_min_distance = glm::min(sum, m_min_distance);
		}
	}

	//float range = max_distance - min_distance;
	//float inverse_range = 1.f / range;

	//for (u32 y = 0; y < size.y; ++y) {
	//	for (u32 x = 0; x < size.x; ++x) {
	//		auto distance = distances[x + y * size.x];
	//		float normalized = (distance - min_distance) * inverse_range;
	//		m_distances[x + y * size.x] = glm::round(normalized * 255.f);
	//	}
	//}
}

std::pair<float, glm::vec2> ShapeSdf::GetDistanceAndGradient(glm::vec2 position) const {
	position *= c_PixelsPerMeter;
	auto clamped_position = glm::clamp(position, glm::vec2(0), glm::vec2(m_size)-1.0001f);
	const glm::ivec2 index = glm::floor(clamped_position);
	const float distance00 = GetDistance(index);
	const float distance10 = GetDistance(index + glm::ivec2(1,0));
	const float distance01 = GetDistance(index + glm::ivec2(0,1));
	const float distance11 = GetDistance(index + glm::ivec2(1,1));
	const glm::vec2 t = glm::fract(clamped_position);
	const float distance0 = glm::mix(distance00, distance10, t.x);
	const float distance1 = glm::mix(distance01, distance11, t.x);
	float distance = glm::mix(distance0, distance1, t.y);

	glm::vec2 gradient;
	if (position == clamped_position) {
		gradient = glm::vec2(distance00) - glm::vec2(distance10, distance01);
		float len = glm::length(gradient);
		if (len < 0.0001f) {
			len = 0.0001f;
		}
		gradient /= len;
	} else {
		gradient = clamped_position - position;
		float len = glm::length(gradient);
		gradient /= len;
		distance += len;
	}

	return { distance, gradient };
}

float ShapeSdf::GetDistance(glm::ivec2 index) const {
	return m_distances[index.x + index.y * m_size.x];
}
