#pragma once

#include <vulkan/vulkan.h>

namespace jgfx {
  struct CreateInfo;
}

namespace jgfx::vk { 
  struct RenderContextVK {
    VkInstance instance;
    // device
    // swap chain

    bool init(const CreateInfo& createInfo);
    void shutdown();
  };
}