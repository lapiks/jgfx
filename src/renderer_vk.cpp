#define VK_USE_PLATFORM_WIN32_KHR

#include "renderer_vk.h"

#include "jgfx/jgfx.h"
#include "utils_vk.h"

#include <set>
#include <iostream>

namespace jgfx::vk {
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
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

    if (enableValidationLayers && !utils::checkValidationLayerSupport(validationLayers)) {
      return false;
    }

    // Application def
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "JGFX";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Instance def
    VkInstanceCreateInfo vkCreateInfo{};
    vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInfo.pApplicationInfo = &appInfo;
    vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(initInfo.extensionNames.size());
    vkCreateInfo.ppEnabledExtensionNames = initInfo.extensionNames.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
      vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      vkCreateInfo.ppEnabledLayerNames = validationLayers.data();

      debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debugCreateInfo.pfnUserCallback = debugCallback;
      vkCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
      vkCreateInfo.enabledLayerCount = 0;
      vkCreateInfo.pNext = nullptr;
    }

    // Instance creation
    if (vkCreateInstance(&vkCreateInfo, nullptr, &_instance) != VK_SUCCESS)
      return false;

    if (enableValidationLayers) {
      if (createDebugUtilsMessengerEXT(&debugCreateInfo, nullptr) != VK_SUCCESS) {
        return false;
      }
    }

    if (!_swapChain.createSurface(_instance, initInfo.platformData))
      return false;

    // We need the swap chain extension for drawing to screen
    const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    if (!pickPhysicalDevice(_swapChain._surface, deviceExtensions))
      return false;

    if (!createLogicalDevice(_swapChain._surface, deviceExtensions))
      return false;

    if (!_swapChain.createSwapChain(_device, _physicalDevice, initInfo.resolution))
      return false;

    if (!_swapChain.createImageViews(_device))
      return false;

    if (!_defaultPass.create(_device, _swapChain._imageFormat))
      return false;

    if (!_swapChain.createFramebuffers(_device, _defaultPass._renderPass))
      return false;

    if (!_cmdQueue.createCommandPool(_device, _physicalDevice, _swapChain._surface))
      return false;

    if (!_cmdQueue.createCommandBuffers(_device))
      return false;

    if (!_cmdQueue.createSyncObjects(_device))
      return false;

    if (!createDescriptorPool())
      return false;

    // TODO: to move
    _uniformBuffers[0].create(_device, _physicalDevice, 1 << 20);

    _swapChain.acquire(_device);

    _cmdQueue.begin();

    return true;
  }

  void RenderContextVK::shutdown() {
     // wait for finishing drawings
    vkDeviceWaitIdle(_device);

    _cmdQueue.destroy(_device);

    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    //for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
    //  _framebuffers[i].destroy(_device);
    //}
    for (int i = 0; i < MAX_PIPELINES; i++) {
      _pipelines[i].destroy(_device);
    }
    for (int i = 0; i < MAX_PASSES; i++) {
      _passes[i].destroy(_device);
    }
    _defaultPass.destroy(_device);
    for (int i = 0; i < MAX_SHADERS; i++) {
      _shaders[i].destroy(_device);
    }
    _swapChain.destroy(_device);
    _swapChain.destroySurface(_instance);

    for (int i = 0; i < MAX_IMAGES; i++) {
      _images[i].destroy(_device);
    }

    for (int i = 0; i < MAX_BUFFERS; i++) {
      _buffers[i].destroy(_device);
    }
    for (int i = 0; i < MAX_BUFFERS; i++) {
      _uniformBuffers[i].destroy(_device);
  }
#ifdef NDEBUG
    // nondebug
#else
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(_instance, _debugMessenger, nullptr);
    }
#endif

    vkDestroyDevice(_device, nullptr);
    vkDestroyInstance(_instance, nullptr);
  }

  bool RenderContextVK::pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      return false;
    }

    // enumarate available physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    // check for the first suitable device
    // todo better selection of physical device
    for (const auto& device : devices) {
      if (utils::isDeviceSuitable(device, surface, deviceExtensions)) {
        _physicalDevice = device;

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        _physicalDeviceFeatures.samplerAnisotropy = supportedFeatures.samplerAnisotropy;

        return true;
      }
    }

    return false;
  }

  bool RenderContextVK::createLogicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
    // Check for available queue families
    QueueFamilyIndices indices = utils::findQueueFamilies(_physicalDevice, surface);

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

    // Logical device def
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &_physicalDeviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // Device level's validation layers are deprecated
    createInfo.enabledLayerCount = 0;

    // Logical device creation
    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
      return false;
    }

    // Get queues created alongside logical device
    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_cmdQueue._graphicsQueue);
    vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_swapChain._presentQueue);

    return true;
  }

  bool RenderContextVK::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  VkResult RenderContextVK::createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
      return func(_instance, pCreateInfo, pAllocator, &_debugMessenger);
    }
    else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void RenderContextVK::updateResolution(const Resolution& resolution) {
    _swapChain._needRecreation = true;
    _swapChain._resolution = resolution;
  }

  void RenderContextVK::newShader(ShaderHandle handle, const void* binData, uint32_t size) {
    _shaders[handle.id].create(
      _device, 
      binData,
      size
    );
  }

  void RenderContextVK::newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type) {
    void* mappedMem;

    VkBufferUsageFlags usage;
    switch (type) {
    case VERTEX_BUFFER: usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      break;
    case INDEX_BUFFER: usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      break;
    case UNIFORM_BUFFER: usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      break;
    }

    // create the buffer and prepare mapped memory
    _buffers[handle.id].create(
      _device,
      _physicalDevice,
      size,
      usage,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // host buffer
      &mappedMem
    );

    // copy data in mapped memory
    memcpy(mappedMem, data, static_cast<size_t>(size));

    // unmap memory
    _buffers[handle.id].unmapMemory(_device);
  }

  void RenderContextVK::newUniformBuffer(UniformBufferHandle handle, uint32_t size) {
    _uniformBuffers[handle.id].create(
      _device,
      _physicalDevice,
      size
    );
  }

  void RenderContextVK::newPipeline(PipelineHandle handle, const PipelineDesc& pipelineDesc) {
    _pipelines[handle.id].create(
      _device, 
      _shaders[pipelineDesc.vs.id], 
      _shaders[pipelineDesc.fs.id],
      _defaultPass,//_passes[pass.id]
      pipelineDesc      
    );

    _currentVertexShader = pipelineDesc.vs;
    _currentFragmentShader = pipelineDesc.fs;
  }

  void RenderContextVK::newPass(PassHandle handle, const PassDesc& passDesc) {
    _passes[handle.id].create(
      _device,
      _swapChain._imageFormat
    );
  }

  void RenderContextVK::newImage(ImageHandle handle, const void* data, uint32_t size, const TextureDesc& desc) {
    _images[handle.id].create(
      _device,
      _physicalDevice,
      _cmdQueue,
      desc.width,
      desc.height,
      data
    );
  }

  void RenderContextVK::beginDefaultPass() {
    _cmdQueue.beginPass(
      _defaultPass._renderPass,
      _swapChain._framebuffers[_swapChain._currentImageIdx]._framebuffer,
      _swapChain._extent
    );
  }

  void RenderContextVK::beginPass(PassHandle pass) {
    _cmdQueue.beginPass(
      _passes[pass.id]._renderPass,
      _swapChain._framebuffers[_swapChain._currentImageIdx]._framebuffer,
      _swapChain._extent
    );
  }

  void RenderContextVK::applyPipeline(PipelineHandle pipe) {
    _cmdQueue.applyPipeline(
      _pipelines[pipe.id]._graphicsPipeline,
      _swapChain._extent
    );

    _currentPipeline = pipe;
  }

  void RenderContextVK::endPass() {
    _cmdQueue.endPass();
  }

  void RenderContextVK::draw(uint32_t firstVertex, uint32_t vertexCount) {
    for (uint32_t i = 0; i < _currentUniformBufferId; i++) {
      _uniformBuffers[_currentUniformBufferId].createDescriptorSets(_device, _descriptorPool, _shaders[_currentVertexShader.id]._descriptorSetLayout, _cmdQueue._currentFrame);
      _cmdQueue.bindDescriptorSets(_pipelines[_currentPipeline.id]._pipelineLayout, _uniformBuffers[i]._descriptorSets);
    }

    _cmdQueue.draw(firstVertex, vertexCount);
  }

  void RenderContextVK::drawIndexed(uint32_t firstIndex, uint32_t indexCount) {
    for (uint32_t i = 0; i < _currentUniformBufferId; i++) {
      _uniformBuffers[i].createDescriptorSets(_device, _descriptorPool, _shaders[_currentVertexShader.id]._descriptorSetLayout, _cmdQueue._currentFrame);
      _cmdQueue.bindDescriptorSets(_pipelines[_currentPipeline.id]._pipelineLayout, _uniformBuffers[i]._descriptorSets);
    }

    _cmdQueue.drawIndexed(firstIndex, indexCount);
  }

  void RenderContextVK::commitFrame() {
    _cmdQueue.end();

    // submit cmds
    _cmdQueue.setWaitSemaphore(_swapChain._imageAvailableSemaphore);
    _cmdQueue.submit();

    // present image
    _swapChain.setWaitSemaphore(_cmdQueue._renderFinishedSemaphores[_cmdQueue._currentFrame]);
    _swapChain.present();

    // starts a new frame
    _cmdQueue.newFrame(_device);

    for (uint32_t i = 0; i < _currentUniformBufferId; i++) {
      vkFreeDescriptorSets(_device, _descriptorPool, 1, &_uniformBuffers[i]._descriptorSets[_cmdQueue._currentFrame]);
    }

    _currentUniformBufferId = 0;

    if (_swapChain._needRecreation)
      _swapChain.update(_device, _physicalDevice, _defaultPass._renderPass);

    _swapChain.acquire(_device);

    _cmdQueue.releaseResources(_device);
    _cmdQueue.begin();
  }

  void RenderContextVK::applyBindings(const Bindings& bindings) {
    VkBuffer vertexBuffers[MAX_BUFFER_BIND];
    for (int i = 0; i < MAX_BUFFER_BIND; i++) {
      vertexBuffers[i] = _buffers[bindings.vertexBuffers[i].id]._buffer;
    }
    VkDeviceSize offsets[] = { 0 };

    _cmdQueue.bindVertexBuffers(0, 1, vertexBuffers);
    _cmdQueue.bindIndexBuffer(_buffers[bindings.indexBuffer.id]._buffer);
  }

  void RenderContextVK::applyUniforms(ShaderStage stage, const void* data, uint32_t size) {
    _uniformBuffers[_currentUniformBufferId].update(data, size, _cmdQueue._currentFrame);
    _currentUniformBufferId++;
  }

  bool SwapChainVK::createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, const Resolution& resolution) {
    // Check what is supported for the swap chain
    SwapChainSupportDetails swapChainSupport = utils::querySwapChainSupport(physicalDevice, _surface);

    // choose form
    VkSurfaceFormatKHR surfaceFormat = utils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    // choose present mode
    VkPresentModeKHR presentMode = utils::chooseSwapPresentMode(swapChainSupport.presentModes);
    // choose extent
    VkExtent2D extent = utils::chooseSwapExtent(swapChainSupport.capabilities, resolution);

    // Recommended image count is at least the min capability + 1
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // check we don't exceed the capabilities' maximum number of images 
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      // clamp to max
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Swap chain def
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // todo: VK_IMAGE_USAGE_TRANSFER_DST_BIT to enable post processing
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    QueueFamilyIndices indices = utils::findQueueFamilies(physicalDevice, _surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // an image can be used by multiple queue families at the same time
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // an image is owned by one queue family at a time
    }

    // Swap chain creation
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
      return false;
    }

    // retrieve swapchain's images
    // get image count
    vkGetSwapchainImagesKHR(device, _swapChain, &imageCount, nullptr);
    _images.resize(imageCount);
    // get actual images
    vkGetSwapchainImagesKHR(device, _swapChain, &imageCount, _images.data());

    // save image format and extent
    _imageFormat = surfaceFormat.format;
    _extent = extent;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore)) {
      return false;
    }

    return true;
  }

  bool SwapChainVK::createSurface(VkInstance instance, const PlatformData& platformData) {
    // Surface def
    // TODO: handling of other platforms than windows
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)platformData.nativeWindowHandle;
    createInfo.hinstance = GetModuleHandle(nullptr);

    // Surface creation
    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &_surface) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  bool SwapChainVK::createImageViews(VkDevice device) {
    // create image views for each image
    _imageViews.resize(_images.size());
    for (size_t i = 0; i < _images.size(); i++) {
      // Image view def
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = _images[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = _imageFormat;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      // Image view creation
      if (vkCreateImageView(device, &createInfo, nullptr, &_imageViews[i]) != VK_SUCCESS) {
        return false;
      }
    }

    return true;
  }

  bool SwapChainVK::createFramebuffers(VkDevice device, VkRenderPass renderPass) {
    // Create framebuffer for each image
    _framebuffers.resize(_images.size());
    for (size_t i = 0; i < _images.size(); i++) {
      VkImageView attachments[] = {
        _imageViews[i]
      };

      if (!_framebuffers[i].create(device, attachments, _extent, renderPass))
        return false;
    }

    return true;
  }

  void SwapChainVK::destroy(VkDevice device) {
    vkDestroySemaphore(device, _imageAvailableSemaphore, nullptr);
    for (auto framebuffer : _framebuffers) {
      framebuffer.destroy(device);
    }
    for (auto imageView : _imageViews) {
      vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, _swapChain, nullptr);
  }

  void SwapChainVK::destroySurface(VkInstance instance) {
    vkDestroySurfaceKHR(instance, _surface, nullptr);
  }

  void SwapChainVK::update(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass) {
    destroy(device);
    createSwapChain(device, physicalDevice, _resolution);
    createImageViews(device);
    createFramebuffers(device, renderPass);

    _needRecreation = false;
  }

  void SwapChainVK::acquire(VkDevice device) {
    VkResult result = vkAcquireNextImageKHR(device, _swapChain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &_currentImageIdx);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      _needRecreation = true;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      return; // todo error handling
    }
  }

  void SwapChainVK::present() {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    VkSemaphore waitSmeaphores[] = { _waitSemaphore };
    presentInfo.pWaitSemaphores = { waitSmeaphores }; // wait for renderFinished semaphore to be signaled

    VkSwapchainKHR swapChains[] = { _swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &_currentImageIdx;

    // submit request to present image to the swap chain
    VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      _needRecreation = true;
    }
    else if (result != VK_SUCCESS) {
      return; // todo error handling;
    }
  }

  void SwapChainVK::setWaitSemaphore(VkSemaphore waitSemaphore) {
    _waitSemaphore = waitSemaphore;
  }

  bool ShaderVK::create(VkDevice device, const void* binData, uint32_t size) {
    // Shader module def
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(binData);

    // Shader module creation
    if (vkCreateShaderModule(device, &createInfo, nullptr, &_module) != VK_SUCCESS) {
      return false;
    }

    return createDescriptorSetLayout(device);
  }

  bool ShaderVK::createDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: to change with corresponding shader stage
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void ShaderVK::destroy(VkDevice device) {
    vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
    vkDestroyShaderModule(device, _module, nullptr);
  }

  bool PipelineVK::create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment, const PassVK& pass, const PipelineDesc& pipelineDesc) {
    // Shader stages:
    // Vertex shader def
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertex._module;
    vertShaderStageInfo.pName = "main";

    // Fragment shader def
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragment._module;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = pipelineDesc.vertexAttributes._stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrDescriptions[MAX_VERTEX_ATTRIBUTES];
    for (int i = 0; i < pipelineDesc.vertexAttributes._attrCount; i++) {
      attrDescriptions[i].binding = 0;
      attrDescriptions[i].location = i;
      attrDescriptions[i].format = utils::toVkFormat(pipelineDesc.vertexAttributes._types[i]);
      attrDescriptions[i].offset = pipelineDesc.vertexAttributes._offsets[i];
    }

    // Vertex input def
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = pipelineDesc.vertexAttributes._attrCount;
    vertexInputInfo.pVertexAttributeDescriptions = attrDescriptions;

    // Primitive def
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = utils::toVkPrimitiveTopology(pipelineDesc.primitive);
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Dynamic state for viewport and scissor
    std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    // Dynamic state def
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer def
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = utils::toVkCullModeFlagBits(pipelineDesc.cullMode);
    rasterizer.frontFace = utils::toVkFrontFace(pipelineDesc.faceWinding);
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling def
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Color blending def
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vertex._descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
      return false;
    }

    // Pipeline def
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = pass._renderPass;
    pipelineInfo.subpass = 0; // which subpass to use
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional. To derive a pass from another.

    // Pipeline creation
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void PipelineVK::destroy(VkDevice device) {
    vkDestroyPipeline(device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
  }

  bool PassVK::create(VkDevice device, VkFormat swapChainImageFormat) {
    // Attachment def
    // Define the attachment format to use but it do not actualy reference an actual image view
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Image to be presented in the swap chain

    // Reference to framebuffer attachment
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Sub pass def
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependencies
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Pass def
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Pass creation
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void PassVK::destroy(VkDevice device) {
    vkDestroyRenderPass(device, _renderPass, nullptr);
  }

  bool BufferVK::create(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void** mappedMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) 
      return false;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, _buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = utils::findMemoryType(
      physicalDevice, 
      memRequirements.memoryTypeBits, 
      properties
    );

    // allocate memory on GPU
    if (vkAllocateMemory(device, &allocInfo, nullptr, &_memory) != VK_SUCCESS) {
      return false;
    }

    vkBindBufferMemory(device, _buffer, _memory, 0);

    // map the buffer memory into CPU accessible memory
    vkMapMemory(device, _memory, 0, bufferInfo.size, 0, mappedMemory);

    _size = size;

    return true;
  }

  void BufferVK::unmapMemory(VkDevice device) {
    vkUnmapMemory(device, _memory);
  }

  void BufferVK::destroy(VkDevice device) {
    vkDestroyBuffer(device, _buffer, nullptr);
    vkFreeMemory(device, _memory, nullptr);
  }

  bool UniformBufferVK::create(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t size) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (!_buffers[i].create(
        device,
        physicalDevice,
        size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &_mappedMemory[i])
      )
        return false;
    }

    return true;
  }

  void UniformBufferVK::update(const void* data, uint32_t size, uint32_t currentFrame) {
    // copy into mapped memory
    memcpy(_mappedMemory[currentFrame], data, size);
    _buffers[currentFrame]._size = size;
  }

  void UniformBufferVK::destroy(VkDevice device) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      _buffers[i].destroy(device);
    }
  }

  bool UniformBufferVK::createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t currentFrame) {
    std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device, &allocInfo, &_descriptorSets[currentFrame]) != VK_SUCCESS) {
      return false;
    }

    //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = _buffers[currentFrame]._buffer;
      bufferInfo.offset = 0;
      bufferInfo.range = _buffers[currentFrame]._size;

      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = _descriptorSets[currentFrame];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;

      vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    //}

    return true;
  }

  bool FramebufferVK::create(VkDevice device, const VkImageView* attachments, VkExtent2D swapChainExtent, VkRenderPass renderPass) {
    // Framebuffer def
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass; // which compatible (same attachments) render pass to use with 
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments; // which attachments (aka image views) to use
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    //Framebuffer creation
    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void FramebufferVK::destroy(VkDevice device) {
    vkDestroyFramebuffer(device, _framebuffer, nullptr);
  }

  void CommandQueueVK::destroy(VkDevice device) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device, _renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(device, _inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, _commandPool, nullptr);
  }

  bool CommandQueueVK::createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    QueueFamilyIndices queueFamilyIndices = utils::findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  bool CommandQueueVK::createCommandBuffers(VkDevice device) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(device, &allocInfo, _commandBuffers) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  bool CommandQueueVK::createSyncObjects(VkDevice device) {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
        return false;
      }
    }

    return true;
  }

  void CommandQueueVK::begin() {
    vkResetCommandBuffer(_commandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(_commandBuffers[_currentFrame], &beginInfo) != VK_SUCCESS) {
      return; // todo error handling
    }
  }

  void CommandQueueVK::end() {
    if (vkEndCommandBuffer(_commandBuffers[_currentFrame]) != VK_SUCCESS) {
      return; // todo error handling
    }
  }

  void CommandQueueVK::beginPass(VkRenderPass pass, VkFramebuffer framebuffer, const VkExtent2D& extent) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; // todo: to be parametrized
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(_commandBuffers[_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  }

  void CommandQueueVK::endPass() {
    vkCmdEndRenderPass(_commandBuffers[_currentFrame]);
  }

  void CommandQueueVK::applyPipeline(VkPipeline pipeline, const VkExtent2D& extent) {
    vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffers[_currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(_commandBuffers[_currentFrame], 0, 1, &scissor);
  }

  void CommandQueueVK::bindDescriptorSets(VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets) {
    vkCmdBindDescriptorSets(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[_currentFrame], 0, nullptr);
  }

  void CommandQueueVK::draw(uint32_t firstVertex, uint32_t vertexCount) {
    vkCmdDraw(_commandBuffers[_currentFrame], vertexCount, 1, firstVertex, 0);
  }

  void CommandQueueVK::drawIndexed(uint32_t firstIndex, uint32_t indexCount) {
    vkCmdDrawIndexed(_commandBuffers[_currentFrame], indexCount, 1, 0, firstIndex, 0);
  }

  void CommandQueueVK::submit() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // wait for image available signal
    VkSemaphore waitSemaphores[] = { _waitSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

    // signal on renderFinished semaphore
    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // will signal inFlightFence when the command queue will finish execution
    if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
      return; // todo error handling
    }
  }

  void CommandQueueVK::newFrame(VkDevice device) {
    // wait to start a new frame
    vkWaitForFences(device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &_inFlightFences[_currentFrame]);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void CommandQueueVK::setWaitSemaphore(VkSemaphore waitSemaphore) {
    _waitSemaphore = waitSemaphore;
  }

  void CommandQueueVK::addResourceToRelease(VkObjectType type, uint64_t handle) {
    _toRelease[_currentFrame].push_back({ type, handle });
  }

  void CommandQueueVK::releaseResources(VkDevice device) {
    for (const Resource& resource : _toRelease[_currentFrame]) {
      switch (resource.type) {
      case VK_OBJECT_TYPE_BUFFER: vkDestroyBuffer(device, VkBuffer(resource.handle), nullptr); break;
      case VK_OBJECT_TYPE_DEVICE_MEMORY: vkFreeMemory(device, VkDeviceMemory(resource.handle), nullptr); break;
      default: // not handled yet
        break;
      }
    }

    _toRelease[_currentFrame].clear();
  }

  void CommandQueueVK::bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* vertexBuffers) {
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(_commandBuffers[_currentFrame], firstBinding, bindingCount, vertexBuffers, offsets);
  }

  void CommandQueueVK::bindIndexBuffer(VkBuffer indexBuffer) {
    vkCmdBindIndexBuffer(_commandBuffers[_currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
  }

  bool ImageVK::create(VkDevice device, VkPhysicalDevice physicalDevice, CommandQueueVK& cmdQueue, uint32_t width, uint32_t height, const void* data) {
    uint64_t imageSize = width * height * 4;

    BufferVK stagingBuffer;
    void* mappedMem = nullptr;
    stagingBuffer.create(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &mappedMem);

    // copy data in mapped memory
    memcpy(mappedMem, data, static_cast<size_t>(imageSize));

    // unmap memory
    stagingBuffer.unmapMemory(device);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device, &imageInfo, nullptr, &_textureImage) != VK_SUCCESS) {
      return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, _textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = utils::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &_deviceMemory) != VK_SUCCESS) {
      return false;
    }

    vkBindImageMemory(device, _textureImage, _deviceMemory, 0);

    VkCommandBuffer cmdBuf = cmdQueue._commandBuffers[cmdQueue._currentFrame];

    // transition for copy
    transitionImageLayout(cmdBuf, _textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // copy staging buffer data to host buffer
    copyBufferToImage(cmdBuf, stagingBuffer._buffer, 1, _textureImage, width, height);

    // transition for shader access
    transitionImageLayout(cmdBuf, _textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    cmdQueue.addResourceToRelease(VK_OBJECT_TYPE_BUFFER, uint64_t(stagingBuffer._buffer));
    cmdQueue.addResourceToRelease(VK_OBJECT_TYPE_DEVICE_MEMORY, uint64_t(stagingBuffer._memory));

    createView(device);

    return true;
  }

  void ImageVK::destroy(VkDevice device) {
    vkDestroySampler(device, _sampler, nullptr);
    vkDestroyImageView(device, _imageView, nullptr);
    vkDestroyImage(device, _textureImage, nullptr);
    vkFreeMemory(device, _deviceMemory, nullptr);
  }

  bool ImageVK::createView(VkDevice device) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = _textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &_imageView) != VK_SUCCESS) {
      return false;
;   }

    return true;
  }

  bool ImageVK::createSampler(VkDevice device, VkPhysicalDevice physicalDevice) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void ImageVK::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, uint32_t bufferImageCopyCount, VkImage image, uint32_t width, uint32_t height) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
      commandBuffer,
      srcBuffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      bufferImageCopyCount,
      &region
    );
  }

  void ImageVK::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
      throw std::invalid_argument("unsupported layout transition");
    }

    vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );
  }

}