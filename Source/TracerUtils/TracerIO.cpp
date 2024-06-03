#define STB_IMAGE_IMPLEMENTATION
#include "TracerIO.hpp"

#include <fstream>
#include <stdexcept>

namespace TracerUtils
{


    std::unique_ptr<std::vector<char>> IOHelpers::ReadFile(const std::string& filePath)
    {
        //ate - seek to the end of the ile
        //binary - read as binary
        auto path = _assetFolder / std::filesystem::path(filePath);
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if(!file.is_open())
        {
            throw std::runtime_error("fieled to open file: " + filePath + " full path: " + path.string());
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        auto buffer = std::make_unique<std::vector<char>>(fileSize);

        file.seekg(0); //go to the start of the file
        file.read(buffer->data(), fileSize);

        return std::move(buffer);
    }

    stbi_uc *IOHelpers::LoadImage(const std::string &filePath, int *width, int *height, int *channels, bool useAlphaChannel)
    {
        auto path = _assetFolder / std::filesystem::path(filePath);
        stbi_uc* pixels = stbi_load(path.string().c_str(), width, height, channels, useAlphaChannel ? STBI_rgb_alpha : STBI_rgb);

        if (!pixels) 
        {
            throw std::runtime_error("failed to load texture image!" + path.string());
        }

        return pixels;
    }

    void IOHelpers::FreeImage(stbi_uc *image)
    {
        stbi_image_free(image);
    }

    Models::TracerMesh IOHelpers::LoadModel(const std::string &filePath)
    {
        Models::TracerMesh mesh;
        Assimp::Importer importer;

        auto strPath = (_assetFolder / std::filesystem::path(filePath)).string();
        const aiScene* scene = importer.ReadFile(strPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        std::vector<const aiMesh*> meshes;
        VisitNode(scene->mRootNode, scene, &meshes);
        for (size_t i = 0; i < meshes.size(); i++)
        {
            ImportMesh(meshes[i], mesh);
        }

        importer.FreeScene();
        return mesh;
    }

    void IOHelpers::VisitNode(const aiNode *node, const aiScene *scene, std::vector<const aiMesh *> *meshes)
    {
        for (size_t i = 0; i < node->mNumMeshes; i++)
        {
            auto mesh = scene->mMeshes[node->mMeshes[i]];
            meshes->push_back(mesh);
        }

        for (size_t i = 0; i < node->mNumChildren; i++)
        {
            VisitNode(node->mChildren[i], scene, meshes);
        }
    }

    void IOHelpers::ImportMesh(const aiMesh* mesh, Models::TracerMesh& tracerMesh)
    {
        uint32_t indexOffset = tracerMesh.Vertices.size();

        for (size_t i = 0; i < mesh->mNumVertices; i++)
        {
            Models::TracerVertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            //TODO fix texture import
            //vertex.TextureCoordinate = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y); 

            tracerMesh.Vertices.push_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            auto face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                tracerMesh.Indices.push_back(indexOffset + face.mIndices[j]);
            }
        }
    }
}