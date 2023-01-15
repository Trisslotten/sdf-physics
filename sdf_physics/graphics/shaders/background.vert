#version 330 core
layout (location = 0) in vec2 in_vertex_pos;

out vec2 frag_position;

uniform vec2 view_position;
uniform vec2 view_scale;

void main() {
    vec4 world_pos = vec4(2.0 * in_vertex_pos - 1.0, 0.99, 1.0);

	frag_position = world_pos.xy / view_scale + view_position;
    gl_Position = world_pos;
}