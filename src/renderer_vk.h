#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"

constexpr int MAX_SHADERS = 512;
constexpr int MAX_PIPELINES = 512;
constexpr int MAX_PASSES = 512;
constexpr int MAX_FRAMEBUFFERS = 512;
constexpr int MAX_BUFFERS = 4<<10;
constexpr int MAX_FRAMES_IN_FLIGHT = 3;

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
    void destroy(VkDevice device);
    void destroySurface(VkInstance instance);
    void update(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass);
    void acquire(VkDevice device);
    void present();
    void setWaitSemaphore(VkSemaphore waitSemaphore);
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
    VkQueue _presentQueue = VK_NULL_HANDLE; // queue supporting presentation operations
    VkFormat _imageFormat;
    VkExtent2D _extent;
    VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE; // signal that an image has been acquired from swap chain and is ready for rendering
    VkSemaphore _waitSemaphore = VK_NULL_HANDLE;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
    std::vector<FramebufferVK> _framebuffers;
    uint32_t _currentImageIdx;
    Resolution _resolution;
    bool _needRecreation = false;
  };

  struct ShaderVK {
    bool create(VkDevice device, const std::vector<char>& bytecode);
    void destroy(VkDevice device);
    VkShaderModule _module = VK_NULL_HANDLE;
  };

  struct PassVK {
    bool create(VkDevice device, VkFormat swapChainImageFormat);
    void destroy(VkDevice device);
    VkRenderPass _renderPass = VK_NULL_HANDLE;
  };

  struct PipelineVK {
    bool create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment, const PassVK& pass, VertexAttributes attr);
    void destroy(VkDevice device);
    VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
  };

  struct BufferVK {
    bool create(VkDevice device, VkPhysicalDevice physicalDevice, const void* data, uint32_t size);
    void destroy(VkDevice device);
    VkBuffer _buffer = VK_NULL_HANDLE;
    VkDeviceMemory _memory = VK_NULL_HANDLE;
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
    void bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* vertexBuffers);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void submit();
    void newFrame(VkDevice device);
    void setWaitSemaphore(VkSemaphore waitSemaphore);
    VkCommandPool _commandPool = VK_NULL_HANDLE;
    VkCommandBuffer _commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkFence _inFlightFences[MAX_FRAMES_IN_FLIGHT]; // wait for frame ending to start a new one
    VkQueue _graphicsQueue = VK_NULL_HANDLE; // queue supporting draw operations
    VkSemaphore _renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT]; // signal that rendering has finished and presentation can happen
    VkSemaphore _waitSemaphore = VK_NULL_HANDLE;
    uint32_t _currentFrame = 0;
  };

  struct RenderContextVK : public RenderContext {
    // Initialization
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;
    bool pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createLogicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    VkResult createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
    void updateResolution(const Resolution& resolution);

    // ObjectVK creation
    void newPipeline(PipelineHandle handle, ShaderHandle vertex, ShaderHandle fragment, PassHandle pass, VertexAttributes attr);
    void newPass(PassHandle handle);
    void newShader(ShaderHandle handle, const std::vector<char>& bytecode) override;
    void newBuffer(BufferHandle handle, const void* data, uint32_t size);

    // cmds
    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void applyBindings(const Bindings& bindings);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void endPass();
    void commitFrame();
    
  private:
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    
    SwapChainVK _swapChain;
    CommandQueueVK _cmdQueue;
    PassVK _defaultPass;
    ShaderVK _shaders[MAX_SHADERS];
    PipelineVK _pipelines[MAX_PIPELINES];
    PassVK _passes[MAX_PASSES];  
    BufferVK _buffers[MAX_BUFFERS];
  };
}