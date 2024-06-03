#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

out vec3 vertex_color;
//uniform float size; //float to manipulate size with imgui slider

void main()
{
	//gl_Position = vec4(size * a_pos.x, size * a_pos.y, size * a_pos.z, 1.0;
	gl_Position = vec4(a_pos, 1.0);
	vertex_color = a_color;
};