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
}