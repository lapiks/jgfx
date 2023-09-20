#include "renderer_vk.h"

#include "jgfx/jgfx.h"

namespace jgfx::vk {
  bool RenderContextVK::init(const CreateInfo& createInfo)
  {
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
    vkCreateInfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&vkCreateInfo, nullptr, &instance);

    return true;
  }

  void RenderContextVK::shutdown()
  {

  }
}