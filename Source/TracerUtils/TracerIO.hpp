#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <stb_image.h>

namespace TracerUtils
{
    class IOHelpers
    {
    public:
        static std::unique_ptr<std::vector<char>> ReadFile(const std::string& filePath);
        
        static stbi_uc* LoadImage(const std::string& filePath, int* width, int* height, int* channels, bool useAlphaChannel);
        static void FreeImage(stbi_uc* image);

        static inline void SetAssetFolder(const std::string& assetFolderPath) { _assetFolder = assetFolderPath; };

    private:
        static inline std::filesystem::path _assetFolder;
    };
}

