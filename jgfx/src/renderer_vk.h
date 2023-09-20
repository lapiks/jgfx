#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace jgfx {
  struct InitInfo;
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

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct RenderContextVK {
    bool init(const InitInfo& createInfo);
    void shutdown();
    bool createSurface(const PlatformData& platformData);
    bool pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createLogicalDevice(const std::vector<const char*>& deviceExtensions);
    bool createSwapChain(const InitInfo& initInfo);

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue; // queue supporting draw operations
    VkQueue presentQueue; // queue supporting presentation operations
    VkSwapchainKHR swapChain;
  };
}