#shader vertex
#version 330 core

layout (location = 0) in vec3 a_pos;

uniform float size;

void main()
{
   gl_Position = vec4(size * a_pos.x, size * a_pos.y, size * a_pos.z, 1.0);
};

#shader fragment
#version 330 core

out vec4 frag_color;

uniform vec4 color; 

void main()
{
   frag_color = color;
};