#pragma once

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace utils {
    inline void check(VkResult result) {
        if (result != VK_SUCCESS) {
            std::cerr << "Vulkan call returned error " << result << '\n';
            exit(result);
        }
    }

    inline void check(bool result) {
        if (!result) {
            std::cerr << "Call returned an error" << '\n';
            exit(result);
        }
    }

    inline std::vector<char> readFile(const std::string& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file.");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}
