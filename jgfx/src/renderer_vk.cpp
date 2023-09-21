#define VK_USE_PLATFORM_WIN32_KHR

#include "renderer_vk.h"

#include "jgfx/jgfx.h"
#include "utils_vk.h"

#include <set>

namespace jgfx::vk {
  bool RenderContextVK::init(const InitInfo& initInfo) {
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> validationLayers = {

      "VK_LAYER_KHRONOS_validation"
    };

    if (enableValidationLayers && !utils::checkValidationLayerSupport(validationLayers)) {
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

    if (!createImageViews())
      return false;

    return true;
  }

  void RenderContextVK::shutdown() {
    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
  }

  bool RenderContextVK::createSurface(const PlatformData& platformData) {
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
      if (utils::isDeviceSuitable(device, surface, deviceExtensions)) {
        physicalDevice = device;
        return true;
      }
    }

    return false;
  }

  bool RenderContextVK::createLogicalDevice(const std::vector<const char*>& deviceExtensions) {
    // Check for available queue families
    QueueFamilyIndices indices = utils::findQueueFamilies(physicalDevice, surface);

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

  bool RenderContextVK::createSwapChain(const InitInfo& initInfo) {
    // Check what is supported for the swap chain
    SwapChainSupportDetails swapChainSupport = utils::querySwapChainSupport(physicalDevice, surface);

    // choose format
    VkSurfaceFormatKHR surfaceFormat = utils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    // choose present mode
    VkPresentModeKHR presentMode = utils::chooseSwapPresentMode(swapChainSupport.presentModes);
    // choose extent
    VkExtent2D extent = utils::chooseSwapExtent(swapChainSupport.capabilities, initInfo);

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

    QueueFamilyIndices indices = utils::findQueueFamilies(physicalDevice, surface);
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

  bool RenderContextVK::createImageViews()
  {
    // create image views for each image
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
        return false;
      }
    }

    return true;
  }

  bool RenderContextVK::createGraphicsPipeline() {
    return true;
  }

  void RenderContextVK::newShader(ShaderHandle handle, const std::vector<char>& bytecode) {
    shaders[handle.id].create(bytecode, device);
  }

  void RenderContextVK::newProgram() {

  }

  bool ShaderVK::create(const std::vector<char>& bytecode, VkDevice device) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &_module) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

}