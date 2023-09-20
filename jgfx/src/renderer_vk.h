#pragma once

#include <vulkan/vulkan.h>

namespace jgfx {
  struct CreateInfo;
}

namespace jgfx::vk { 
  struct RenderContextVK {
    bool init(const CreateInfo& createInfo);
    void shutdown();
    bool pickPhysicalDevice();
    bool createLogicalDevice();

  private:
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
  };
}