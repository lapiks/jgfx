#pragma once

#include <vulkan/vulkan.h>

#include <optional>

namespace jgfx {
  struct CreateInfo;
  struct PlatformData;
}

namespace jgfx::vk { 
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily; // Graphic queue family index
    std::optional<uint32_t> presentFamily; // Surface presentation queue family index

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct RenderContextVK {
    bool init(const CreateInfo& createInfo);
    void shutdown();
    bool createSurface(const PlatformData& platformData);
    bool pickPhysicalDevice(VkSurfaceKHR surface);
    bool createLogicalDevice();

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
  };
}