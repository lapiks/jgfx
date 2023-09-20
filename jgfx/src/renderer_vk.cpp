#define VK_USE_PLATFORM_WIN32_KHR

#include "renderer_vk.h"

#include "jgfx/jgfx.h"

#include <vector>
#include <optional>

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

  std::optional<uint32_t> findQueueFamilies(VkPhysicalDevice device) {
    std::optional<uint32_t> indices;
    // Logic to find queue family indices to populate struct with

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices = i;
      }

      i++;
    }

    return indices;
  }

  bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    std::optional<uint32_t> indices = findQueueFamilies(device);

    return indices.has_value();
  }

  bool RenderContextVK::init(const CreateInfo& createInfo) {
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
    vkCreateInfo.enabledExtensionCount = createInfo.extensionCount;
    vkCreateInfo.ppEnabledExtensionNames = createInfo.extensionNames;

    if (enableValidationLayers) {
      vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      vkCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
      vkCreateInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&vkCreateInfo, nullptr, &instance);

    if (!createSurface(createInfo.platformData))
      return false;

    if (!pickPhysicalDevice())
      return false;

    if (!createLogicalDevice())
      return false;

    return true;
  }

  void RenderContextVK::shutdown() {
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
  }

  bool RenderContextVK::pickPhysicalDevice() {
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
      if (isDeviceSuitable(device)) {
        physicalDevice = device;
        return true;
      }
    }

    return false;
  }

  bool RenderContextVK::createLogicalDevice()
  {
    // Check for available queue types
    std::optional<uint32_t> indices = findQueueFamilies(physicalDevice);

    // Queue creation
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.value();
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // define needed features
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Logical device creation
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    // Device level's validation layers are deprecated
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
      return false;
    }

    // Get queue created alongside logical device
    vkGetDeviceQueue(device, indices.value(), 0, &graphicsQueue);

    return true;
  }
}