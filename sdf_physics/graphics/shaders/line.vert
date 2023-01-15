#version 330 core
layout (location = 0) in float in_vertex_pos;

uniform vec2 line_start;
uniform vec2 line_end;

uniform vec2 view_position;
uniform vec2 view_scale;

void main() {
	vec2 world_pos = mix(line_start, line_end, in_vertex_pos);
	gl_Position = vec4(view_scale * (world_pos - view_position), 0.1, 1.0);
}
