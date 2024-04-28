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
}