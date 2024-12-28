#include "Model.h"

Model::Model() : loading_in_progress(false), loaded(false)
{

}

Model::~Model()
{
	if (load_thread.joinable())
	{
		load_thread.join();
	}
}

void Model::CheckGPUResources()
{
	if (loaded && !gpu_resources_created)
	{
		CreateGPUResources();
		gpu_resources_created = true;
	}
}

void Model::LoadAsync(const std::string& path)
{
    if (loading_in_progress) {
        std::cerr << "Model is already loading!\n";
        return;
    }

    loading_in_progress = true;
    loaded = false;
    gpu_resources_created = false;

    //start the worker thread
    load_thread = std::thread([this, path]() 
    {
        CPU_Model_Data data = LoadModelData(path);

        //copy to member variable
        {
            std::lock_guard<std::mutex> lock(this->cpu_data_mutex);
            this->cpu_data = std::move(data);
        }

        loading_in_progress = false;
        loaded = true;
    });
}

//when done loading, create GPU buffers and textures on main thread
void Model::CreateGPUResources()
{
	if (gpu_resources_created)
	{
		return;
	}
    //lock cpu data
	std::lock_guard<std::mutex> lock(cpu_data_mutex);

    {
		std::lock_guard<std::mutex> lock(gpu_meshes_mutex);
		gpu_meshes.clear();
    }

	//for each cpu mesh create a mesh (sets up VAO, VBO, EBO)
    //and now textures!
    for (auto& cpu_mesh : cpu_data.meshes)
    {
        for (auto& texture : cpu_mesh.textures) {
            if (texture.is_loaded && texture.id == 0) {
                //generate OpenGL texture
                glGenTextures(1, &texture.id);
                glBindTexture(GL_TEXTURE_2D, texture.id);

                GLenum format;
                if (texture.channels == 1)
                    format = GL_RED;
                else if (texture.channels == 3)
                    format = GL_RGB;
                else if (texture.channels == 4)
                    format = GL_RGBA;
                else
                    format = GL_RGB; //fallback

                glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE, texture.data.data());
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        }

        Mesh mesh(
            cpu_mesh.vertices,
            cpu_mesh.indices,
            cpu_mesh.textures,
            cpu_mesh.material
        );
        {
            std::lock_guard<std::mutex> lock(gpu_meshes_mutex);
            gpu_meshes.push_back(mesh);
        }
    }

	gpu_resources_created = true;
	std::cout << "GPU resources created for model" << std::endl;
}

void Model::Draw(Shader& shader)
{
	if (!gpu_resources_created)
	{
		//CheckGPUResources();
        if (!gpu_resources_created)
        {
            return;
        }
	}

	//lock gpu meshes to prevent changes while drawing
	std::lock_guard<std::mutex> mesh_lock(gpu_meshes_mutex);
	for (auto& mesh : gpu_meshes)
	{
		mesh.Draw(shader);
	}
}

//functions on worker thread

CPU_Model_Data Model::LoadModelData(const std::string& path)
{
    std::cout << "Loading model from: " << path << std::endl;

    CPU_Model_Data result;
	Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate
        | aiProcess_FlipUVs
        | aiProcess_GenSmoothNormals);
        //|aiProcess_OptimizeMeshes //further optimization, can disable for performance
        //|aiProcess_OptimizeGraph //can disable for performance
        //|aiProcess_JoinIdenticalVertices //can disable for performance
        //|aiProcess_ValidateDataStructure //can disable for performance

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return result;
    }

    this->directory = path.substr(0, path.find_last_of('/'));

    //recursively process nodes
    ProcessNode(scene->mRootNode, scene, result);

    return result;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, CPU_Model_Data& model_data)
{
    //process each mesh at this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, model_data);
    }
    //process children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, model_data);
    }
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, CPU_Model_Data& model_data)
{
    CPU_Mesh cpu_mesh;
    cpu_mesh.vertices.reserve(mesh->mNumVertices);
    cpu_mesh.indices.reserve(mesh->mNumFaces * 3); //triangulated faces due to assimp triangulate flag

    //store vertices in vertices vector
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		//store texture coords
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            vertex.tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y);
        }
        else 
        {
            vertex.tex_coords = glm::vec2(0.0f, 0.0f);
        }

        cpu_mesh.vertices.push_back(vertex);
    }

    ////go through each face (which are triangles due to aiProcess_Triangulate) and store indices in the indices vector
    for (unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        const aiFace& face = mesh->mFaces[f];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            cpu_mesh.indices.push_back(face.mIndices[j]);
        }
    }

    //mesh contains index to a material object. to get the material of a mesh we index the scene's mMaterials array.
    //the mesh's material index is set in its mMaterialIndex property which we can query to check if the mesh contains a material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        cpu_mesh.material = LoadMaterial(material);

        //load diffuse textures
        unsigned int diffuse_count = material->GetTextureCount(aiTextureType_DIFFUSE);
        for (unsigned int i = 0; i < diffuse_count; i++)
        {
            Texture tex = LoadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse", i);
            cpu_mesh.textures.push_back(tex);
        }

        //load specular textures
        unsigned int specular_count = material->GetTextureCount(aiTextureType_SPECULAR);
        for (unsigned int i = 0; i < specular_count; i++)
        {
            Texture tex = LoadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular", i);
            cpu_mesh.textures.push_back(tex);
        }

        //if no textures found, create a fallback white texture
        if (diffuse_count == 0 && specular_count == 0) 
        {
            //fallback white texture
            Texture fallback_texture = CreateWhiteTexture();
            cpu_mesh.textures.push_back(fallback_texture);
        }
    }
    else {
        //default material
        cpu_mesh.material.ambient = glm::vec3(0.2f);
        cpu_mesh.material.diffuse = glm::vec3(0.8f);
        cpu_mesh.material.specular = glm::vec3(1.0f);
        cpu_mesh.material.shininess = 32.0f;

        //fallback white texture
        Texture fallback_texture = CreateWhiteTexture();
        cpu_mesh.textures.push_back(fallback_texture);
    }

    std::cout << "Loaded mesh: " << cpu_mesh.vertices.size() << " vertices, " << cpu_mesh.indices.size() << " indices\n";

    model_data.meshes.push_back(std::move(cpu_mesh));
}


//Texture and material functions

Texture Model::LoadMaterialTexture(aiMaterial* material, aiTextureType type, const std::string& type_name,
    unsigned int index)
{
    Texture texture;
    aiString str;
    material->GetTexture(type, index, &str);

    std::string full_path = directory + "/" + str.C_Str();
    texture.path = full_path;
	texture.type = type_name;

    //check cache first
    auto it = texture_cache.find(full_path);
    if (it != texture_cache.end()) {
        texture = it->second;
    }
    else {
        //load and then....
        if (LoadTextureData(str.C_Str(), directory, texture)) {
            //store in cache
            texture_cache[full_path] = texture;
        }
        else {
            //DO SOMETHING!
        }
    }
    return texture;
}

/*unsigned int Model::TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = directory + '/' + std::string(path);

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
        else
            format = GL_RGB; //fallback

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
        std::cerr << "Texture failed to load: " << filename << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}*/

bool Model::LoadTextureData(const char* path, const std::string& directory, Texture& texture)
{
	std::string filename = directory + '/' + std::string(path);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		texture.data.assign(data, data + (width * height * nrChannels));
		texture.width = width;
		texture.height = height;
		texture.channels = nrChannels;
		texture.is_loaded = true;
		stbi_image_free(data);
		return true;
	}
	else
	{
		std::cerr << "Texture failed to load: " << filename << std::endl;
		stbi_image_free(data);
		texture.is_loaded = false;
		return false;
	}
}


Material Model::LoadMaterial(aiMaterial* _material)
{
    Material material;
    aiColor3D color(0.f, 0.f, 0.f);
    float shininess = 0.0f;

    //diffuse
    if (_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        material.diffuse = glm::vec3(color.r, color.g, color.b);
    }
    else {
        material.diffuse = glm::vec3(0.8f);
    }

    //ambient
    if (_material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        material.ambient = glm::vec3(color.r, color.g, color.b);
    }
    else {
        material.ambient = glm::vec3(0.2f);
    }

    //specular
    if (_material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        material.specular = glm::vec3(color.r, color.g, color.b);
    }
    else {
        material.specular = glm::vec3(1.0f);
    }

    //shininess
    if (_material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        material.shininess = shininess;
    }
    else {
        material.shininess = 32.0f;
    }

    return material;
}

Texture Model::CreateWhiteTexture()
{
    Texture texture;
    texture.type = "texture_diffuse";
    texture.path = "fallback_white";

    //prepare 1x1 white pixel data
    texture.width = 1;
    texture.height = 1;
    texture.channels = 4; // RGBA
    texture.data = { 255, 255, 255, 255 };
    texture.is_loaded = true;

    return texture;
}