#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/shader_manager.h"
#include "Material.h"

#include <string>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

struct Texture {
	//unsigned int id; //async loading means we can't do opengl calls on other threads
	//only main threads. so we cant use opengl texture id anymore
	std::string type; //diffuse, specular etc.
	std::string path;

	std::vector<unsigned char> data;
	int width = 0;
	int height = 0;
	int channels = 0;
	bool is_loaded = false;
	unsigned int id = 0; //we will use opengl texture ID on functions called on main thread

	//bindless texture handle
	uint64_t bindless_handle = 0;
};

class Mesh
{
public:
	//mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	Material material;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, const Material& material)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->material = material;

		SetupMesh();
	}

	void Draw(Shader& shader)
	{
		//set the texture handle as a uniform
		for (const auto& texture : textures)
		{
			if (texture.type == "texture_diffuse")
			{
				// Set the bindless texture handle as a uniform
				shader.SetUint64("material_diffuse", texture.bindless_handle);
			}
			else if (texture.type == "texture_specular")
			{
				shader.SetUint64("material_specular", texture.bindless_handle);
			}
		}

		shader.SetVec3("material.diffuse_color", material.diffuse);
		shader.SetVec3("material.ambient_color", material.ambient);
		shader.SetVec3("material.specular_color", material.specular);
		shader.SetFloat("material.shininess", material.shininess);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	~Mesh()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

private:
	//render data
	unsigned int VAO, VBO, EBO;
	void SetupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		//vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		//vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

		glBindVertexArray(0);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "OpenGL Error during SetupMesh: " << error << std::endl;
		}
	}
};

#endif