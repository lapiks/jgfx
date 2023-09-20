#pragma once

#include <vulkan/vulkan.h>

namespace jgfx {
  struct CreateInfo;
  struct PlatformData;
}

namespace jgfx::vk { 
  struct RenderContextVK {
    bool init(const CreateInfo& createInfo);
    void shutdown();
    bool createSurface(const PlatformData& platformData);
    bool pickPhysicalDevice();
    bool createLogicalDevice();

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
  };
}