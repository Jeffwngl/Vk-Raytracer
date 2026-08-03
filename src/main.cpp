#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <assert.h>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANDS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

const uint32_t maxFramesInFlight{ 2 };

