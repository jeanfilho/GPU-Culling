#include "stdafx.h"
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::unique_ptr<Model> ModelLoader::LoadModel(const std::string& filePath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filePath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_PreTransformVertices
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return nullptr;
    }

    auto model = std::make_unique<Model>();

    ProcessNode(scene->mRootNode, scene, model.get());

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        auto material = ProcessMaterial(scene->mMaterials[i], std::filesystem::path(filePath).parent_path().string());
        if (material)
        {
            model->materials.push_back(*material);
        }
    }

    CalculateBoundingBox(model.get());

    return model;
}

bool ModelLoader::IsFileSupported(const std::string& filePath) const
{
    const char* supportedExtensions[] = {
        ".obj", ".gltf", ".glb", ".fbx", ".dae", ".blend", ".3ds", ".ase", ".ply", ".dxf"
    };

    std::string extension = std::filesystem::path(filePath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return std::tolower(c); });

    for (const char* ext : supportedExtensions)
    {
        if (extension == ext)
        {
            return true;
        }
    }

    return false;
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, Model* outModel)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto processedMesh = ProcessMesh(mesh, scene);
        if (processedMesh)
        {
            outModel->meshes.push_back(*processedMesh);
        }
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene, outModel);
    }
}

std::unique_ptr<Mesh> ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    auto processedMesh = std::make_unique<Mesh>();
    processedMesh->name = mesh->mName.C_Str();
    processedMesh->materialIndex = mesh->mMaterialIndex;

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex{};

        vertex.position = XMFLOAT3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        if (mesh->HasNormals())
        {
            vertex.normal = XMFLOAT3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }
        else
        {
            vertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        }

        if (mesh->mTextureCoords[0])
        {
            vertex.texCoord = XMFLOAT2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }
        else
        {
            vertex.texCoord = XMFLOAT2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = XMFLOAT3(
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z
            );

            vertex.bitangent = XMFLOAT3(
                mesh->mBitangents[i].x,
                mesh->mBitangents[i].y,
                mesh->mBitangents[i].z
            );
        }
        else
        {
            vertex.tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
            vertex.bitangent = XMFLOAT3(0.0f, 1.0f, 0.0f);
        }

        processedMesh->vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j)
        {
            processedMesh->indices.push_back(face.mIndices[j]);
        }
    }

    return processedMesh;
}

std::unique_ptr<Material> ModelLoader::ProcessMaterial(aiMaterial* material, const std::string& modelDir)
{
    auto processedMaterial = std::make_unique<Material>();
    processedMaterial->name = material->GetName().C_Str();

    aiColor3D diffuse(0.8f, 0.8f, 0.8f);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    processedMaterial->diffuse = XMFLOAT3(diffuse.r, diffuse.g, diffuse.b);

    aiColor3D specular(0.5f, 0.5f, 0.5f);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    processedMaterial->specular = XMFLOAT3(specular.r, specular.g, specular.b);

    aiColor3D ambient(0.2f, 0.2f, 0.2f);
    material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    processedMaterial->ambient = XMFLOAT3(ambient.r, ambient.g, ambient.b);

    float shininess = 32.0f;
    material->Get(AI_MATKEY_SHININESS, shininess);
    processedMaterial->shininess = shininess;

    aiString texturePath;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
        processedMaterial->diffuseTexture = std::string(modelDir) + "/" + texturePath.C_Str();
    }

    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &texturePath);
        processedMaterial->normalTexture = std::string(modelDir) + "/" + texturePath.C_Str();
    }

    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath);
        processedMaterial->specularTexture = std::string(modelDir) + "/" + texturePath.C_Str();
    }

    return processedMaterial;
}

void ModelLoader::CalculateBoundingBox(Model* outModel)
{
    if (outModel->meshes.empty())
    {
        outModel->boundingBoxMin = XMFLOAT3(0.0f, 0.0f, 0.0f);
        outModel->boundingBoxMax = XMFLOAT3(0.0f, 0.0f, 0.0f);
        return;
    }

    XMFLOAT3 minBounds(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 maxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& mesh : outModel->meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            minBounds.x = std::min(minBounds.x, vertex.position.x);
            minBounds.y = std::min(minBounds.y, vertex.position.y);
            minBounds.z = std::min(minBounds.z, vertex.position.z);

            maxBounds.x = std::max(maxBounds.x, vertex.position.x);
            maxBounds.y = std::max(maxBounds.y, vertex.position.y);
            maxBounds.z = std::max(maxBounds.z, vertex.position.z);
        }
    }

    outModel->boundingBoxMin = minBounds;
    outModel->boundingBoxMax = maxBounds;
}

void ModelLoader::CalculateTangentSpace(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        XMVECTOR pos0 = XMLoadFloat3(&v0.position);
        XMVECTOR pos1 = XMLoadFloat3(&v1.position);
        XMVECTOR pos2 = XMLoadFloat3(&v2.position);

        XMVECTOR edge1 = XMVectorSubtract(pos1, pos0);
        XMVECTOR edge2 = XMVectorSubtract(pos2, pos0);

        XMFLOAT2 uv0 = v0.texCoord;
        XMFLOAT2 uv1 = v1.texCoord;
        XMFLOAT2 uv2 = v2.texCoord;

        float deltaU1 = uv1.x - uv0.x;
        float deltaV1 = uv1.y - uv0.y;
        float deltaU2 = uv2.x - uv0.x;
        float deltaV2 = uv2.y - uv0.y;
        
        float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

        XMVECTOR tangent;
        tangent = XMVectorScale(edge1, deltaV2);
        tangent = XMVectorAdd(tangent, XMVectorScale(edge2, -deltaV1));
        tangent = XMVectorScale(tangent, f);
        tangent = XMVector3Normalize(tangent);

        XMFLOAT3 tangentFloat3;
        XMStoreFloat3(&tangentFloat3, tangent);
        v0.tangent = tangentFloat3;
        v1.tangent = tangentFloat3;
        v2.tangent = tangentFloat3;

        XMVECTOR normal0 = XMLoadFloat3(&v0.normal);
        XMVECTOR normal1 = XMLoadFloat3(&v1.normal);
        XMVECTOR normal2 = XMLoadFloat3(&v2.normal);

        XMVECTOR bitangent0 = XMVector3Cross(normal0, tangent);
        XMVECTOR bitangent1 = XMVector3Cross(normal1, tangent);
        XMVECTOR bitangent2 = XMVector3Cross(normal2, tangent);

        XMFLOAT3 bitangentFloat3_0, bitangentFloat3_1, bitangentFloat3_2;
        XMStoreFloat3(&bitangentFloat3_0, bitangent0);
        XMStoreFloat3(&bitangentFloat3_1, bitangent1);
        XMStoreFloat3(&bitangentFloat3_2, bitangent2);

        v0.bitangent = bitangentFloat3_0;
        v1.bitangent = bitangentFloat3_1;
        v2.bitangent = bitangentFloat3_2;
    }
}
