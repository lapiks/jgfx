#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

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
}