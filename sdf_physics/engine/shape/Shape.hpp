#pragma once

#include <vector>
#include "util/IntTypes.hpp"
#include <glm/vec2.hpp>
#include "ShapeId.hpp"

constexpr u8 c_MaterialEmptySpace = 0;

class ShapeSdf {
public:
	void Create(const std::vector<u8>& image, glm::uvec2 size);

	/// Gradient is pointing towards the surface
	std::pair<float, glm::vec2> GetDistanceAndGradient(glm::vec2 position) const;
	float GetDistance(glm::ivec2 index) const;
	std::vector<float> m_distances;
private:
	glm::uvec2 m_size;
	float m_min_distance;
	float m_max_distance;
};

class Shape {
public:
	Shape();
	Shape(glm::uvec2 size);

	const std::vector<u8>& GetImage() const;
	const glm::uvec2& GetSize() const;

	u8 GetPixelAt(glm::uvec2 pixel) const;

	glm::vec2 GetSizeInMeters() const;

	void SetCenterOffset(glm::vec2 offset);
	const glm::vec2& GetCenterOffset() const;

	const ShapeSdf& GetSdf() const;

	ShapeId GetId() const;
	void SetId(ShapeId);
private:
	void GenerateRandomShape();

	std::vector<u8> m_image;
	glm::uvec2 m_size;
	glm::vec2 m_center_offset;
	ShapeId m_id;

	ShapeSdf m_sdf;
};
