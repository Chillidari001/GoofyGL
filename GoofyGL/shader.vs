#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;
layout (location = 2) in vec2 a_tex_coord;

out vec3 vertex_color;
out vec2 tex_coord;
//uniform float size; //float to manipulate size with imgui slider

uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	//gl_Position = vec4(size * a_pos.x, size * a_pos.y, size * a_pos.z, 1.0;
	//gl_Position = transform * vec4(a_pos, 1.0);
	gl_Position = projection * view * model * vec4(a_pos, 1.0);
	//vertex_color = a_color;
	//tex_coord = a_tex_coord;
	tex_coord = vec2(a_tex_coord.x, a_tex_coord.y);
};