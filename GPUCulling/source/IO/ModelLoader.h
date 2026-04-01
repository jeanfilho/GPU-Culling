#pragma once

#include <vector>
#include <string>
#include <memory>
#include <DirectXMath.h>
#include <assimp/scene.h>

using namespace DirectX;

struct VertexData
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 texCoord;
    XMFLOAT3 tangent;
    XMFLOAT3 bitangent;
};

struct MeshData
{
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;
    std::string name;
    uint32_t materialIndex = 0;
};

struct MaterialData
{
    std::string name;
    XMFLOAT3 diffuse = XMFLOAT3(0.8f, 0.8f, 0.8f);
    XMFLOAT3 specular = XMFLOAT3(0.5f, 0.5f, 0.5f);
    XMFLOAT3 ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);
    float shininess = 32.0f;
    std::string diffuseTexture;
    std::string normalTexture;
    std::string specularTexture;
};

struct ModelData
{
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
    XMFLOAT3 boundingBoxMin;
    XMFLOAT3 boundingBoxMax;
};

class ModelLoader
{
public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    ModelLoader(const ModelLoader&) = delete;
    ModelLoader& operator=(const ModelLoader&) = delete;
    ModelLoader(ModelLoader&&) = default;
    ModelLoader& operator=(ModelLoader&&) = default;

    std::unique_ptr<ModelData> LoadModel(const std::string& filePath);
    bool IsFileSupported(const std::string& filePath) const;

private:
    void ProcessNode(aiNode* node, const aiScene* scene, ModelData* outModel);
    std::unique_ptr<MeshData> ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::unique_ptr<MaterialData> ProcessMaterial(aiMaterial* material, const std::string& modelDir);
    void CalculateBoundingBox(ModelData* outModel);
    void CalculateTangentSpace(std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices);
};
