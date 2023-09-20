#define VK_USE_PLATFORM_WIN32_KHR

#include "renderer_vk.h"

#include "jgfx/jgfx.h"

#include <vector>
#include <set>
#include <string>
#include <limits>
#include <algorithm>
#include <cstdint>

namespace jgfx::vk {
  bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    // Logic to find queue family indices to populate struct with

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      // check if the queue family has support for graphics
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }

      // check if the queue family has support for surface presentation
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (presentSupport) {
        indices.presentFamily = i;
      }

      if (indices.isComplete())
        return indices;

      i++;
    }

    return indices;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Check for SRGB8 color format
    for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const InitInfo& initInfo) {
    // Compile error here :/
    /*if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    }
    else */ {
      VkExtent2D actualExtent = {
          static_cast<uint32_t>(initInfo.resolutionWidth),
          static_cast<uint32_t>(initInfo.resolutionHeight)
      };

      actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

      return actualExtent;
    }
  }

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
  }

  /// <summary>
  /// Check if the given device supports the requiredExtensions 
  /// </summary>
  /// <param name="device"></param>
  /// <param name="requiredExtensions"></param>
  /// <returns></returns>
  bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

    for (const auto& extension : availableExtensions) {
      requiredExtensionsSet.erase(extension.extensionName);
    }

    return requiredExtensionsSet.empty();
  }

  bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device, requiredExtensions);

    bool swapChainSupported = false;
    if (extensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
      swapChainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainSupported;
  }

  bool RenderContextVK::init(const InitInfo& initInfo) {
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> validationLayers = {

      "VK_LAYER_KHRONOS_validation"
    };

    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
      return false;
    }

    // Instance creation
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo vkCreateInfo{};
    vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInfo.pApplicationInfo = &appInfo;
    vkCreateInfo.enabledExtensionCount = initInfo.extensionCount;
    vkCreateInfo.ppEnabledExtensionNames = initInfo.extensionNames;

    if (enableValidationLayers) {
      vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      vkCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
      vkCreateInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&vkCreateInfo, nullptr, &instance);

    if (!createSurface(initInfo.platformData))
      return false;

    // We need the swap chain extension for drawing to screen
    const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    if (!pickPhysicalDevice(surface, deviceExtensions))
      return false;

    if (!createLogicalDevice(deviceExtensions))
      return false;

    if (!createSwapChain(initInfo))
      return false;

    return true;
  }

  void RenderContextVK::shutdown() {
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
  }

  bool RenderContextVK::createSurface(const PlatformData& platformData)
  {
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)platformData.nativeWindowHandle;
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  bool RenderContextVK::pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      return false;
    }

    // enumarate available physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // check for the first suitable device
    for (const auto& device : devices) {
      if (isDeviceSuitable(device, surface, deviceExtensions)) {
        physicalDevice = device;
        return true;
      }
    }

    return false;
  }

  bool RenderContextVK::createLogicalDevice(const std::vector<const char*>& deviceExtensions)
  {
    // Check for available queue families
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    // Queues creation
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    // For each unique queue family
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      // Create an info struct
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    // define needed features
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Logical device creation
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // Device level's validation layers are deprecated
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
      return false;
    }

    // Get queues created alongside logical device
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    return true;
  }

  bool RenderContextVK::createSwapChain(const InitInfo& initInfo)
  {
    // Check what is supported for the swap chain
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    // choose format
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    // choose present mode
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    // choose extent
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, initInfo);

    // Recommended image count is at least the min capability + 1
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // check we don't exceed the capabilities' maximum number of images 
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      // clamp to max
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Swap chain construction
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // todo: VK_IMAGE_USAGE_TRANSFER_DST_BIT to enable post processing

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // an image can be used by multiple queue families at the same time
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // an image is owned by one queue family at a time
      createInfo.queueFamilyIndexCount = 0; // Optional
      createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
      return false;
    }

    // retrieve swapchain's images
    // get image count
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    // get actual images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    // save image format and extent
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    return true;
  }
}