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
		unsigned int diffuseNumber = 1;
		unsigned int specularNumber = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); //get active texture unit before binding
			//get texture number (diffuse_texterNUMBER)

			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				std::cerr << "OpenGL Error during glActiveTexture in Draw: " << error << std::endl;
			}

			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
			{
				number = std::to_string(diffuseNumber++);
			}
			else if (name == "texture_specular")
			{
				number = std::to_string(specularNumber++);
			}
			shader.SetInt(("material." + name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
			error = glGetError();
			if (error != GL_NO_ERROR)
			{
				std::cerr << "OpenGL Error during glBindTexture in Draw: " << error << std::endl;
			}
		}
		glActiveTexture(GL_TEXTURE0);

		shader.SetVec3("material.diffuse_color", material.diffuse);
		shader.SetVec3("material.ambient_color", material.ambient);
		shader.SetVec3("material.specular_color", material.specular);
		shader.SetFloat("material.shininess", material.shininess);

		//draw the mesh
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