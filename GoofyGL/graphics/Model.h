#include "../core/shader_manager.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../external/stb_image.h"
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>

#ifndef MODEL_H
#define MODEL_H

struct CPU_Mesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	Material material;
};

struct CPU_Model_Data
{
	std::vector<CPU_Mesh> meshes;
	std::unordered_map<std::string, Texture> texture_cache;
};

class Model
{
public:
    Model();
    ~Model();

    //start loading on a background thread.
    void LoadAsync(const std::string& path);

    //once loading is complete, call this on the main thread with a valid OpenGL context
    //to create the GPU buffers/textures.
    void CreateGPUResources();

    //true if the background thread has finished loading.
    bool IsLoaded() const { return loaded; }
	bool IsGPUResourcesCreated() const { return gpu_resources_created; } //what am i doing rn

    void CheckGPUResources();

    void Draw(Shader& shader);

private:
    //worker thread function that does the heavy CPU-bound loading.
    CPU_Model_Data LoadModelData(const std::string& path);

    // Process the Assimp scene, fill out CPU_ModelData
    void ProcessNode(aiNode* node, const aiScene* scene, CPU_Model_Data& model_data);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene, CPU_Model_Data& model_data);

    // Load textures on CPU, but also create the GL texture (which is an OpenGL call).
    // If you truly want no GL calls in the worker thread, you can skip this step here
    // and only decode raw data, then finalize textures in CreateGPUResources().
    Texture LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string& type_name, unsigned int index);
    //unsigned int TextureFromFile(const char* path, const std::string& directory);
    bool LoadTextureData(const char* path, const std::string& directory, Texture& texture);

    //fallback placeholder texture if texture is not found
    static Texture CreateWhiteTexture();

    Material LoadMaterial(aiMaterial* mat);

    //threading flags
    std::thread load_thread;
    std::atomic<bool> loading_in_progress;
    std::atomic<bool> loaded;
    std::atomic<bool> gpu_resources_created; 

    //CPU data populated by load_thread
    CPU_Model_Data cpu_data;
    std::mutex cpu_data_mutex; // protect cpu_data

    //GPU data (final Mesh objects with VAOs, VBOs, EBOs)
    std::vector<Mesh> gpu_meshes;
    std::mutex gpu_meshes_mutex;

    //stores loaded textures so we dont load them again
    std::unordered_map<std::string, Texture> texture_cache;

    //path for model files
    std::string directory;
};

#endif