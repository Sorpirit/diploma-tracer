#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_write.h>

#include "Models/TracerMesh.hpp"
#include "Models/TracerVertex.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

namespace TracerUtils
{
    class IOHelpers
    {
    public:
        static std::unique_ptr<std::vector<char>> ReadFile(const std::string& filePath);
        
        static stbi_uc* LoadImage(const std::string& filePath, int* width, int* height, int* channels, bool useAlphaChannel);
        static void SaveImage(const std::string& filePath, stbi_uc* image, int width, int height, int channels);
        static void FreeImage(stbi_uc* image);
        static Models::TracerMesh LoadModel(const std::string& filePath);

        static inline void SetAssetFolder(const std::string& assetFolderPath) { _assetFolder = assetFolderPath; };

    private:
        static inline std::filesystem::path _assetFolder;

        static void VisitNode(const aiNode* node, const aiScene* scene, std::vector<const aiMesh*>* meshes);
        static void ImportMesh(const aiMesh* mesh, Models::TracerMesh& tracerMesh);
    };
}
