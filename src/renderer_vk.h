#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"

constexpr int MAX_SHADERS = 512;
constexpr int MAX_PIPELINES = 512;
constexpr int MAX_PASSES = 512;
constexpr int MAX_FRAMEBUFFERS = 512;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

namespace jgfx {
  struct InitInfo;
  struct PlatformData;
}

namespace jgfx::vk { 
  struct FramebufferVK {
    bool create(VkDevice device, const VkImageView* attachments, VkExtent2D swapChainExtent, VkRenderPass renderPass);
    void destroy(VkDevice device);
    VkFramebuffer _framebuffer;
  };

  struct SwapChainVK {
    bool createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, const Resolution& resolution);
    bool createSurface(VkInstance instance, const PlatformData& platformData);
    bool createImageViews(VkDevice device);
    bool createFramebuffers(VkDevice device, VkRenderPass renderPass);
    void destroy(VkDevice device, VkInstance instance);
    void acquire(VkDevice device);
    void present();
    void setWaitSemaphore(VkSemaphore waitSemaphore);
    VkSurfaceKHR _surface;
    VkSwapchainKHR _swapChain;
    VkQueue _presentQueue; // queue supporting presentation operations
    VkFormat _imageFormat;
    VkExtent2D _extent;
    VkSemaphore _imageAvailableSemaphore; // signal that an image has been acquired from swap chain and is ready for rendering
    VkSemaphore _waitSemaphore;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
    std::vector<FramebufferVK> _framebuffers;
    uint32_t _currentImageIdx;
  };

  struct ShaderVK {
    bool create(VkDevice device, const std::vector<char>& bytecode);
    void destroy(VkDevice device);
    VkShaderModule _module;
  };

  struct PassVK {
    bool create(VkDevice device, VkFormat swapChainImageFormat);
    void destroy(VkDevice device);
    VkRenderPass _renderPass;
  };

  struct PipelineVK {
    bool create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment, const PassVK& pass);
    void destroy(VkDevice device);
    VkPipeline _graphicsPipeline;
    VkPipelineLayout _pipelineLayout;
  };

  struct CommandQueueVK {
    void begin();
    void end();
    bool createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    bool createCommandBuffers(VkDevice device);
    bool createSyncObjects(VkDevice device);
    void destroy(VkDevice device);
    void beginPass(VkRenderPass pass, VkFramebuffer framebuffer, const VkExtent2D& extent);
    void endPass();
    void applyPipeline(VkPipeline pipeline, const VkExtent2D& extent);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void submit();
    void newFrame(VkDevice device);
    void setWaitSemaphore(VkSemaphore waitSemaphore);
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkFence _inFlightFences[MAX_FRAMES_IN_FLIGHT]; // wait for frame ending to start a new one
    VkQueue _graphicsQueue; // queue supporting draw operations
    VkSemaphore _renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT]; // signal that rendering has finished and presentation can happen
    VkSemaphore _waitSemaphore;
    uint32_t _currentFrame = 0;
  };

  struct RenderContextVK : public RenderContext {
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;
    bool pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createLogicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    VkResult createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);

    void newPipeline(PipelineHandle handle, ShaderHandle vertex, ShaderHandle fragment, PassHandle pass);
    void newPass(PassHandle handle);
    void newShader(ShaderHandle handle, const std::vector<char>& bytecode) override;

    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void endPass();
    void commitFrame();
    
  private:
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;
    
    SwapChainVK _swapChain;
    CommandQueueVK _cmdQueue;
    PassVK _defaultPass;
    ShaderVK _shaders[MAX_SHADERS];
    PipelineVK _pipelines[MAX_PIPELINES];
    PassVK _passes[MAX_PASSES];  
  };
}