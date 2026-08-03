#include <iostream>
#include <vulkan/vulkan_core.h>

namespace utils {
    void check(VkResult result) {
        if (result != VK_SUCCESS) {
            std::cerr << "Vulkan call returned error " << result << '\n';
            exit(result);
        }
    }

    void check(bool result) {
        if (!result) {
            std::cerr << "Call returned an error" << '\n';
            exit(result);
        }
    }


}
