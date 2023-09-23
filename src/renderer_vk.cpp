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

    // Application def
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Instance def
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

    // Instance creation
    VkResult result = vkCreateInstance(&vkCreateInfo, nullptr, &_instance);

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

    if (!_swapChain.createFramebuffers(_device))
      return false;

    if (!_cmdQueue.createCommandPool(_device, _physicalDevice, _swapChain._surface))
      return false;

    if (!_cmdQueue.createCommandBuffer(_device))
      return false;

    if (!_cmdQueue.createSyncObjects(_device))
      return false;

    _cmdQueue.begin();

    return true;
  }

  void RenderContextVK::shutdown() {
    _cmdQueue.destroy(_device);
    //for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
    //  _framebuffers[i].destroy(_device);
    //}
    for (int i = 0; i < MAX_PIPELINES; i++) {
      _pipelines[i].destroy(_device);
    }
    for (int i = 0; i < MAX_PASSES; i++) {
      _passes[i].destroy(_device);
    }
    for (int i = 0; i < MAX_SHADERS; i++) {
      _shaders[i].destroy(_device);
    }
    vkDestroyDevice(_device, nullptr);
    _swapChain.destroy(_device, _instance);
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
    for (const auto& device : devices) {
      if (utils::isDeviceSuitable(device, surface, deviceExtensions)) {
        _physicalDevice = device;
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

    // define needed features
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Logical device def
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // Device level's validation layers are deprecated
    createInfo.enabledLayerCount = 0;

    // Logical device creation
    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
      return false;
    }

    // Get queues created alongside logical device
    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

    return true;
  }

  void RenderContextVK::newShader(ShaderHandle handle, const std::vector<char>& bytecode) {
    _shaders[handle.id].create(
      _device, 
      bytecode
    );
  }

  void RenderContextVK::newPipeline(PipelineHandle handle, ShaderHandle vertex, ShaderHandle fragment, PassHandle pass) {
    _pipelines[handle.id].create(
      _device, 
      _shaders[vertex.id], 
      _shaders[fragment.id],
      _passes[pass.id]
    );
  }

  void RenderContextVK::newPass(PassHandle handle)
  {
    _passes[handle.id].create(
      _device,
      _swapChain._imageFormat
    );
  }

  void RenderContextVK::beginPass(PassHandle pass, uint32_t imageIndex) {
    _cmdQueue.beginPass(
      _passes[pass.id]._renderPass,
      _swapChain._framebuffers[imageIndex]._framebuffer,
      _swapChain._extent
    );
  }

  void RenderContextVK::applyPipeline(PipelineHandle pipe) {
    _cmdQueue.applyPipeline(
      _pipelines[pipe.id]._graphicsPipeline,
      _swapChain._extent
    );
  }

  void RenderContextVK::endPass() {
    _cmdQueue.endPass();
  }

  void RenderContextVK::draw(uint32_t firstVertex, uint32_t vertexCount) {
    _cmdQueue.draw(firstVertex, vertexCount);
  }

  void RenderContextVK::commitFrame() {
    _swapChain.acquire(_device);
    _cmdQueue.end();
    _cmdQueue.begin();
    
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
      createInfo.queueFamilyIndexCount = 0; // Optional
      createInfo.pQueueFamilyIndices = nullptr; // Optional
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

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFence) != VK_SUCCESS) {
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

  bool SwapChainVK::createFramebuffers(VkDevice device) {
    // Create framebuffer for each image
    _framebuffers.resize(_images.size());
    for (size_t i = 0; i < _images.size(); i++) {
      _framebuffers[i].create(device, _imageViews, _extent);
    }

    return true;
  }

  void SwapChainVK::destroy(VkDevice device, VkInstance instance) {
    for (auto framebuffer : _framebuffers) {
      framebuffer.destroy(device);
    }
    for (auto imageView : _imageViews) {
      vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, _swapChain, nullptr);
    vkDestroySurfaceKHR(instance, _surface, nullptr);
  }

  void SwapChainVK::acquire(VkDevice device) {
    // wait for previous frame to finish
    vkWaitForFences(device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, _swapChain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
  }

  bool ShaderVK::create(VkDevice device, const std::vector<char>& bytecode) {
    // Shader module def
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

    // Shader module creation
    if (vkCreateShaderModule(device, &createInfo, nullptr, &_module) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void ShaderVK::destroy(VkDevice device) {
    vkDestroyShaderModule(device, _module, nullptr);
  }

  bool PipelineVK::create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment, const PassVK& pass) {
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

    // Vertex input def
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // Primitive def
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
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
    pipelineInfo.basePipelineIndex = -1; // Optional. To derive a pass from another.

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

    // Pass def
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // Pass creation
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  void PassVK::destroy(VkDevice device) {
    vkDestroyRenderPass(device, _renderPass, nullptr);
  }

  bool FramebufferVK::create(VkDevice device, const std::vector<VkImageView>& attachments, VkExtent2D swapChainExtent, VkRenderPass renderPass) {
    // Framebuffer def
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass; // which compatible (same attachments) render pass to use with 
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data(); // which attachments (aka image views) to use
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    //Framebuffer creation
    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
      return false;
    }
  }

  void FramebufferVK::destroy(VkDevice device) {
    vkDestroyFramebuffer(device, _framebuffer, nullptr);
  }

  void CommandQueueVK::destroy(VkDevice device) {
    vkDestroySemaphore(device, _imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, _renderFinishedSemaphore, nullptr);
    vkDestroyFence(device, _inFlightFence, nullptr);
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

  bool CommandQueueVK::createCommandBuffer(VkDevice device) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &_commandBuffer) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  bool CommandQueueVK::createSyncObjects(VkDevice device) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore) != VK_SUCCESS ||
      vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore) != VK_SUCCESS ||
      vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFence) != VK_SUCCESS) {
      return false;
    }
  }

  void CommandQueueVK::begin() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS) {
      return; // todo error handling
    }
  }

  void CommandQueueVK::end() {
    vkResetCommandBuffer(_commandBuffer, 0);
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

    vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  }

  void CommandQueueVK::endPass() {
    vkCmdEndRenderPass(_commandBuffer);

    if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
      return; // todo error handling
    }
  }

  void CommandQueueVK::applyPipeline(VkPipeline pipeline, const VkExtent2D& extent) {
    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
  }

  void CommandQueueVK::draw(uint32_t firstVertex, uint32_t vertexCount) {
    vkCmdDraw(_commandBuffer, vertexCount, 1, firstVertex, 0);
  }
}