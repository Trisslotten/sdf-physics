#include "ShapeTextureManager.hpp"

#include <glad/glad.h>

#include "engine/shape/Shape.hpp"

void ShapeTexture::Bind(u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

void ShapeTextureManager::CreateTexture(ShapeId id, const Shape& shape) {
	ShapeTexture texture;

	glGenTextures(1, &texture.texture_id);

	glBindTexture(GL_TEXTURE_2D, texture.texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto& size = shape.GetSize();
	texture.size = size;

	//glTexImage2D(
	//	GL_TEXTURE_2D,
	//	0,
	//	GL_R8,
	//	size.x,
	//	size.y,
	//	0,
	//	GL_RED,
	//	GL_UNSIGNED_BYTE,
	//	shape.GetImage().data()
	//);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R32F,
		size.x,
		size.y,
		0,
		GL_RED,
		GL_FLOAT,
		//shape.GetImage().data()
		shape.GetSdf().m_distances.data()
	);

	// glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture

	m_shape_textures.emplace(id, std::move(texture));
}

ShapeTexture* ShapeTextureManager::GetTexture(ShapeId id) {
	if (auto iter = m_shape_textures.find(id); iter != m_shape_textures.end()) {
		return &iter->second;
	}
	return nullptr;
}

void ShapeTextureManager::DeleteTexture(ShapeId id) {
	if (auto iter = m_shape_textures.find(id); iter != m_shape_textures.end()) {
		// remove texture in gl
		m_shape_textures.erase(iter);
	}
}

bool ShapeTextureManager::IsTextureUploaded(ShapeId id) const {
	return m_shape_textures.contains(id);
}
