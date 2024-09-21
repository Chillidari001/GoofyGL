#version 330 core
//in vec4 vertex_color; //vertex colour from vertex shader
out vec4 frag_color;
//uniform vec4 color; //vec4 to manipulate color with imgui (also uniforms are basically global variables)
//"uniform vec4 time_color;"
in vec3 vertex_color;
in vec2 tex_coord;

uniform sampler2D container_texture;
uniform sampler2D face_texture;

void main()
{
//   frag_color = color; //colour == colour set in imgui
//	  frag_color = time_color;
     frag_color = mix(texture(container_texture, tex_coord), texture(face_texture, tex_coord), 0.2);
};