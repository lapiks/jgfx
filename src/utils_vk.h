#pragma once

#include "structs_vk.h"

namespace jgfx {
  struct InitInfo;
  struct Resolution;
  enum AttribType;
  enum CullMode;
  enum PrimitiveType;
  enum FaceWinding;
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
  VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
  uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

  // toVk conversions
  VkFormat toVkFormat(AttribType type);
  VkCullModeFlagBits toVkCullModeFlagBits(CullMode mode);
  VkPrimitiveTopology toVkPrimitiveTopology(PrimitiveType type);
  VkFrontFace toVkFrontFace(FaceWinding faceWinding);
}