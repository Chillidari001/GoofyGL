#include "../core/shader_manager.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../external/stb_image.h"

#ifndef MODEL_H
#define MODEL_H

class Model
{
public:

	std::vector<Texture> loaded_textures;

	Model(const std::string &path)
	{
		LoadModel(path);
	}
	void Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(shader);
		}
	}
private:
	//model data
	std::vector<Mesh> meshes;
	std::string directory;

	void LoadModel(std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}
	void ProcessNode(aiNode* node, const aiScene* scene)
	{
		//process all the node's meshes
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}
		//do the same for its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.tex_coords = vec;
			}
			else
			{
				vertex.tex_coords = glm::vec2(0.0f, 0.0f);
			}
			
			vertices.push_back(vertex);
		}
		//go through each face (which are triangles due to aiProcess_Triangulate) and store the indices in the indices vector
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		//mesh contains index to a material object. to get the material of a mesh we index the scene's mMaterials array.
		//the mesh's material index is set in its mMaterialIndex property which we can query to check if the mesh contains a material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuse_maps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
			std::vector<Texture> specular_maps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
		}

		return Mesh(vertices, indices, textures);
	}
	std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string type_name)
	{
		//check if texture is already loaded and in loaded textures vector, if it is skip if not load and then add to vector
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < loaded_textures.size(); j++)
			{
				if (std::strcmp(loaded_textures[j].path.c_str(), str.C_Str()) == 0)
				{
					textures.push_back(loaded_textures[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = type_name;
				texture.path = str.C_Str();
				textures.push_back(texture);
				loaded_textures.push_back(texture);
			}
		}
		return textures;
	}
	
	//uses stb_image to load a texture
	unsigned int TextureFromFile(const char* path, const std::string& directory)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
	
};
#endif

