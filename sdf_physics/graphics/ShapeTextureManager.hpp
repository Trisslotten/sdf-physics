#pragma once

#include <glm/vec2.hpp>
#include <robin_hood/robin_hood.h>
#include "engine/shape/ShapeId.hpp"
#include "util/IntTypes.hpp"

class Shape;

struct ShapeTexture {
	void Bind(u32 slot);

	glm::uvec2 size;
	u32 texture_id;
};

class ShapeTextureManager {
public:
	void CreateTexture(ShapeId, const Shape& shape);

	ShapeTexture* GetTexture(ShapeId);

	void DeleteTexture(ShapeId);

	bool IsTextureUploaded(ShapeId) const;
private:
	robin_hood::unordered_flat_map<ShapeId, ShapeTexture> m_shape_textures;
};