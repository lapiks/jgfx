#pragma once

#include "structs_vk.h"

namespace jgfx {
  struct InitInfo;
  struct Resolution;
}

namespace jgfx::vk::utils {
  bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Resolution& resolution);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions);
  bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);
}