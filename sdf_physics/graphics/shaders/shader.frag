#version 330 core
out vec4 out_frag_color;

in vec2 texture_coords;

uniform sampler2D u_mat_tex;
uniform vec3 u_color;

vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }
float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

void main() {
//	vec3 pixel_size = vec3(1,1,0) / vec3(textureSize(object_texture, 0), 1.0);
//	vec2 t = mod(texture_coords, pixel_size.xy) / pixel_size.xy;
//	t = smoothstep(0.45,0.55,t);
//	float color00 = texture(object_texture, texture_coords).r;
//	float color10 = texture(object_texture, texture_coords + pixel_size.xz).r;
//	float color01 = texture(object_texture, texture_coords + pixel_size.zy).r;
//	float color11 = texture(object_texture, texture_coords + pixel_size.xy).r;
//	float color0 = mix(color00, color10, t.x);
//	float color1 = mix(color01, color11, t.x);
//	float color = mix(color0, color1, t.y);

	float color00 = texture(u_mat_tex, texture_coords).r;

	vec3 pixel_size = vec3(1,1,0) / vec3(textureSize(u_mat_tex, 0), 1.0);
	float color10 = texture(u_mat_tex, texture_coords + pixel_size.xz).r;
	float color01 = texture(u_mat_tex, texture_coords + pixel_size.zy).r;
	float dx = color10 - color00;
	float dy = color01 - color00;
	out_frag_color = vec4(0.01 * vec2(dx, dy) / pixel_size.xy, 0, 1.0);
	if (color00 > 0.0)
		discard;
	out_frag_color = vec4(vec3(1-0.5*abs(color00)), 1.0);

	//float material = texture(u_mat_tex, texture_coords).r;
	//if (material == 0.0) {
	//	discard;
	//}
	//float brightness = sqrt(0.5 + 0.5*snoise(4.0*texture_coords));
	//out_frag_color = vec4(u_color * brightness, 1.0);
}
