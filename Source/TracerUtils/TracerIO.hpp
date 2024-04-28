#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

namespace TracerUtils
{
    class IOHelpers
    {
    public:
        static std::unique_ptr<std::vector<char>> ReadFile(const std::string& filePath);
        static inline void SetAssetFolder(const std::string& assetFolderPath) { _assetFolder = assetFolderPath; };

    private:
        static inline std::filesystem::path _assetFolder;
    };
}

