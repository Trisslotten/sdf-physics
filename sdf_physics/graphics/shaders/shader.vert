#version 330 core
layout (location = 0) in vec2 in_vertex_pos;

out vec2 texture_coords;

uniform vec2 view_position;
uniform vec2 view_scale;

uniform vec2 u_size;
uniform vec2 u_corner;
uniform float u_rotation;

void main() {
	float s = sin(u_rotation);
	float c = cos(u_rotation);
	mat2 rot = mat2(c, s, -s, c);

	vec2 world_pos = rot * (u_size * in_vertex_pos) + u_corner;

	texture_coords = in_vertex_pos;

    gl_Position = vec4(view_scale * (world_pos - view_position), 0.5, 1.0);
}
